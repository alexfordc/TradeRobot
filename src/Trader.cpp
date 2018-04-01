#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <stdlib.h>
#include <time.h>
#include "Trader.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "RobotFactory.h"
#include "TimeUtil.h"
#include "Logger.h"
#include "Global.h"
#include "Contract.h"
#include "OrderMsg.h"
#include "Util.h"

using namespace std;
using namespace rapidjson;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

Trader* Trader::m_pInstance = nullptr;

Trader& Trader::Instance() {
	if (m_pInstance != nullptr) {
		return *m_pInstance;
	}

	m_pInstance = new Trader();

	return *m_pInstance;
}

Trader::Trader() {
	m_pTraderApi = nullptr;
	mTimeElapsed = 0;
	mState = TS_SLEEP;
	mPositionLimit = 10000;

	// sim account
	Account* pSimNowAccout = new Account();

	strcpy(pSimNowAccout->mBrokerID, "your broker id");
	strcpy(pSimNowAccout->mInvestorID, "your investor id");
	strcpy(pSimNowAccout->mPassword, "your password");

	pSimNowAccout->mFrontAddressMD.push_back(strcpy(new char[32], "tcp://ip:port"));
	pSimNowAccout->mFrontAddressMD.push_back(strcpy(new char[32], "tcp://ip:port"));
	pSimNowAccout->mFrontAddressMD.push_back(strcpy(new char[32], "tcp://ip:port"));

	pSimNowAccout->mFrontAddressTD.push_back(strcpy(new char[32], "tcp://ip:port"));
	pSimNowAccout->mFrontAddressTD.push_back(strcpy(new char[32], "tcp://ip:port"));
	pSimNowAccout->mFrontAddressTD.push_back(strcpy(new char[32], "tcp://ip:port"));

	mAccounts["your sim account"] = pSimNowAccout;

	// real account
	Account* pSHZQAcount = new Account();

	strcpy(pSHZQAcount->mBrokerID, "your broker id");
	strcpy(pSHZQAcount->mInvestorID, "your investor id");
	strcpy(pSHZQAcount->mPassword, "your password");

	pSHZQAcount->mFrontAddressMD.push_back(strcpy(new char[32], "tcp://ip:port"));
	pSHZQAcount->mFrontAddressMD.push_back(strcpy(new char[32], "tcp://ip:port"));

	pSHZQAcount->mFrontAddressTD.push_back(strcpy(new char[32], "tcp://ip:port"));
	pSHZQAcount->mFrontAddressTD.push_back(strcpy(new char[32], "tcp://ip:port"));

	mAccounts["your real account"] = pSHZQAcount;
}

Trader::~Trader() {

}

bool Trader::canSendRequest() {
	return TimeUtil::Instance().getTime() - mRequestTime >= seconds(2);
}

bool Trader::reqInsertOrder(CThostFtdcInputOrderField* req) {
	strcpy(req->BrokerID, mAccounts[mInvestorID]->mBrokerID);
	strcpy(req->InvestorID, mAccounts[mInvestorID]->mInvestorID);

	mRequestTime = TimeUtil::Instance().getTime();

	int iResult = m_pTraderApi->ReqOrderInsert(req, mRequestID++);
	if (iResult != 0) {
		Logger::Error() << "请求录入报单失败! Result: " >> iResult;
		return false;
	}

	return true;
}

bool Trader::reqCancelOrder(CThostFtdcInputOrderActionField* req) {
	strcpy(req->BrokerID, mAccounts[mInvestorID]->mBrokerID);
	strcpy(req->InvestorID, mAccounts[mInvestorID]->mInvestorID);

	mRequestTime = TimeUtil::Instance().getTime();

	int iResult = m_pTraderApi->ReqOrderAction(req, mRequestID++);
	if (iResult != 0) {
		Logger::Error() << "请求撤单失败! Result: " >> iResult;
		return false;
	}

	return true;
}

bool Trader::reqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mAccounts[mInvestorID]->mBrokerID);
	strcpy_s(req.InvestorID, mAccounts[mInvestorID]->mInvestorID);
	int iResult = m_pTraderApi->ReqQryTradingAccount(&req, mRequestID++);

	if (iResult != 0) {
		Logger::Error() << "请求查询账户信息失败! Result: " >> iResult;
		return false;
	}

	return true;
}

void Trader::parseContract() {
	string text;
	Util::readFile("./config/ConfigContract.json", text);

	Document doc;

	doc.Parse(text.c_str());

	for (Value::ValueIterator it = doc.GetArray().Begin(); it != doc.GetArray().End(); ++it) {
		Value& v = *it;
		Contract* pContract = mMarket.addContract(v["Name"].GetString(), v["TimeKey"].GetString(), 
			v["TickPrice"].GetDouble(), v["ShangHai"].GetBool(), v["Quantity"].GetInt(), v["Margin"].GetDouble());
	}

	mMarket.read();
}

void Trader::parseRobot() {
	string text;
	Util::readFile("./config/ConfigRobot.json", text);

	Document doc;
	doc.Parse(text.c_str());

	for (Value::ValueIterator it = doc.GetArray().Begin(); it != doc.GetArray().End(); ++it) {
		Value& rb = *it;
		Contract* pContract = mMarket.getContract(rb["Contract"].GetString());
		Robot* pRobot = RobotFactory::create(rb["Robot"].GetString(), rb["ID"].GetInt(), rb["Params"]);
		pContract->addRobot(pRobot);
		mRobots.push_back(pRobot);
	}
}

void Trader::parseMisc() {
	string text;
	if (Util::readFile("./config/ConfigMisc.json", text)) {
		Document doc;
		doc.Parse(text.c_str());

		if (doc.HasMember("PositionLimit")) {
			mPositionLimit = doc["PositionLimit"].GetInt();
		}
		if (doc.HasMember("Money")) {
			mMoney = doc["Money"].GetDouble();
		}
		if (doc.HasMember("LotsRatio")) {
			mLotsRatio = doc["LotsRatio"].GetDouble();
		}
		if (doc.HasMember("Param1")) {
			mParam1 = doc["Param1"].GetInt();
		}
		if (doc.HasMember("Param2")) {
			mParam2 = doc["Param2"].GetInt();
		}
	}
}

bool Trader::init(const string& investorID) {
	if (mAccounts.find(investorID) == mAccounts.end()) {
		return false;
	}

	mInvestorID = investorID;

	parseContract();
	parseRobot();
	parseMisc();

	if (m_ppInstrumentID == nullptr) {
		size_t num = mMarket.getContracts().size();
		m_ppInstrumentID = new char*[num];

		int i = 0;
		const std::map<std::string, Contract*>& contracts = mMarket.getContracts();
		for (auto it = contracts.begin(); it != contracts.end(); ++it) {
			const std::string& name = it->second->getName();
			char* instrument = new char[name.size() + 1];
			m_ppInstrumentID[i] = instrument;
			strcpy(instrument, name.c_str());
			++i;
		}
	}

	m_pTraderApi = nullptr;
	m_pMarketApi = nullptr;
	m_pTraderSpi = nullptr;
	m_pMarketSpi = nullptr;
	mRequestID = 1;
	mOrderRef = 1;
	mLastTradingDay = gregorian::date(min_date_time);
	mRequestTime = TimeUtil::Instance().getTime();

	srand((unsigned)time(NULL));

	return true;
}

void Trader::update(long long dt) {
	TimeUtil::Instance().update();

	long long lastTime = mTimeElapsed;
	mTimeElapsed += dt;
	if (mTimeElapsed/3600000 > lastTime/3600000) {
		Logger::Info() << "XRobot Running... State:" << mState << ". PC:" >> posCount();
	}

	if (mState == TS_SLEEP) {
		if (taskSleep()) {
			Logger::Info() >> "Enter State: TS_INIT_API.";
			mState = TS_INIT_API;
		}
	}
	else if (mState == TS_INIT_API) {
		if (taskInitAPI()) {
			Logger::Info() >> "Enter State: TS_INIT_API_WAITING.";
			mState = TS_INIT_API_WAITING;
		}
	}
	else if (mState == TS_INIT_API_WAITING) {
		if (taskInitAPIWaiting()) {
			Logger::Info() >> "Enter State: TS_LOGIN.";
			mState = TS_LOGIN;
		}
	}
	else if (mState == TS_LOGIN) {
		if (taskLogin()) {
			Logger::Info() >> "Enter State: TS_LOGIN_WAITING.";
			mState = TS_LOGIN_WAITING;
		}
	}
	else if (mState == TS_LOGIN_WAITING) {
		if (taskLoginWaiting()) {
			Logger::Info() >> "Enter State: TS_PREPARED_WAITING.";
			mState = TS_PREPARED_WAITING;
		}
	}
	else if (mState == TS_PREPARED_WAITING) {
		if (taskPreparedWaiting()) {
			Logger::Info() >> "Enter State: TS_TRADING.";
			mState = TS_TRADING;
		}
	}
	else if (mState == TS_TRADING) {
		if (taskTrading()) {
			Logger::Info() >> "Enter State: TS_LOGOUT.";
			mState = TS_LOGOUT;
		}
	}
	else if (mState == TS_LOGOUT) {
		if (taskLogout()) {
			Logger::Info() >> "Enter State: TS_LOGOUT_WAITING.";
			mState = TS_LOGOUT_WAITING;
		}
	}
	else if (mState == TS_LOGOUT_WAITING) {
		if (taskLogoutWaiting()) {
			Logger::Info() >> "Enter State: TS_RECORD.";
			mState = TS_RECORD;
		}
	}
	else if (mState == TS_RECORD) {
		if (taskRecord()) {
			Logger::Info() >> "Enter State: TS_SLEEP.";
			mState = TS_SLEEP;
		}
	}
}

bool Trader::taskSleep() {
	if (TimeUtil::Instance().isWorkingTime()) {
		return true;
	}

	return false;
}

bool Trader::taskInitAPI() {
	string text;
	if (Util::readFile("./config/Record/Trader.json", text)) {
		Document doc;
		doc.Parse(text.c_str());

		mLastTradingDay = from_undelimited_string(doc["TradingDay"].GetString());
		mOrderRef = doc["OrderRef"].GetInt();
		mMarket.readDoc(doc["Market"]);

		Value& robotData = doc["Robots"];
		for (auto i = mRobots.begin(); i != mRobots.end(); ++i) {
			Value::ConstMemberIterator it = robotData.FindMember((*i)->getKey().c_str());
			if (it != robotData.MemberEnd()) {
				(*i)->readDoc(it->value);
			}
		}
	}

	createAPIInstance();

	return true;
}

bool Trader::taskInitAPIWaiting() {
	return Global::getStatusTD()==Global::TS_CONNECTED && Global::getStatusMD()==Global::MS_CONNECTED;
}

bool Trader::taskLogin() {
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(CThostFtdcReqUserLoginField));
	strcpy(req.BrokerID, mAccounts[mInvestorID]->mBrokerID);
	strcpy(req.UserID, mAccounts[mInvestorID]->mInvestorID);
	strcpy(req.Password, mAccounts[mInvestorID]->mPassword);

	int iRstTd = m_pTraderApi->ReqUserLogin(&req, mRequestID++);
	int iRstMd = m_pMarketApi->ReqUserLogin(&req, mRequestID++);

	if (iRstTd != 0) {
		Logger::Error() << "Trader ReqLogin Failed! Result: " >> iRstTd;
	}

	if (iRstMd != 0) {
		Logger::Error() << "Market ReqLogin Failed! Result: " >> iRstMd;
	}

	return true;
}

bool Trader::taskLoginWaiting() {
	if (Global::getStatusTD() == Global::TS_LOGIN && Global::getStatusMD() == Global::MS_LOGIN) {
		CThostFtdcSettlementInfoConfirmField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, mAccounts[mInvestorID]->mBrokerID);
		strcpy(req.InvestorID, mAccounts[mInvestorID]->mInvestorID);

		int iRstTd = m_pTraderApi->ReqSettlementInfoConfirm(&req, mRequestID++);
		if (iRstTd != 0) {
			Logger::Error() << "投资者结算结果确认请求失败! Result: " >> iRstTd;
		}

		int iRstMd = m_pMarketApi->SubscribeMarketData(m_ppInstrumentID, mMarket.getContracts().size());
		if (iRstMd != 0) {
			Logger::Error() << "发送行情订阅请求失败! Result: " >> iRstMd;
		}

		return true;
	}

	return false;
}

bool Trader::taskPreparedWaiting() {
	if (Global::getStatusTD() == Global::TS_PREPARED && Global::getStatusMD() == Global::MS_PREPARED) {
		TimeUtil::Instance().adjustTime(Global::getLoginTime());
		if (mLastTradingDay < Global::getTradingDay()) {
			mLastTradingDay = Global::getTradingDay();
			mMarket.resetTradingDay();
		}
		return true;
	}

	return false;
}

bool Trader::canOpen() {
	return posCount() < mPositionLimit;
}

int Trader::posCount() {
	int posCount = 0;
	for (auto it = mRobots.begin(); it != mRobots.end(); ++it) {
		if ((*it)->position() != 0) {
			posCount++;
		}
	}

	return posCount;
}

bool Trader::taskTrading() {
	while (!Global::isMsgEmpty()) {
		const OrderMsg* pMsg = Global::popMsg();
		Contract* pContract = mMarket.getContract(pMsg->InstrumentID);
		if (pContract != nullptr) {
			pContract->rspOrderMsg(pMsg);
		}
		delete pMsg;
	}

	mMarket.checkOrder();

	while (!Global::isTickEmpty()) {
		TickData* pTick = Global::popTick();
		Contract* pContract = mMarket.getContract(pTick->InstrumentID);

		if (TimeUtil::Instance().isTradingTime(pContract->getTimeKey(), pTick->Time)) {
			pContract->onTick(pTick);
		}

		delete pTick;
	}

	mMarket.onRobotTick();
	onTimer();

	if (!TimeUtil::Instance().isWorkingTime()) {
		return true;
	}

	return false;
}

void Trader::onTimer() {
	for (auto it = mRobots.begin(); it != mRobots.end(); ++it) {
		(*it)->onTimer(TimeUtil::Instance().getTime());
	}
}

bool Trader::taskLogout() {
	if (m_pTraderSpi != nullptr) {
		m_pTraderApi->Release();
		m_pTraderApi = nullptr;
		delete m_pTraderSpi;
		m_pTraderSpi = nullptr;
	}

	if (m_pMarketSpi != nullptr) {
		m_pMarketApi->Release();
		m_pMarketApi = nullptr;
		delete m_pMarketSpi;
		m_pMarketSpi = nullptr;
	}

	return true;
}

bool Trader::taskLogoutWaiting() {
	if (m_pMarketSpi == nullptr && m_pTraderSpi == nullptr) {
		return true;
	}

	return false;
}

bool Trader::taskRecord() {
	Global::reset();
	mMarket.save();

	Document doc;
	doc.SetObject();
	Document::AllocatorType& allocator = doc.GetAllocator();

	doc.AddMember("TradingDay", Value(to_iso_string(mLastTradingDay).c_str(), allocator), allocator);
	doc.AddMember("OrderRef", mOrderRef, allocator);
	
	doc.AddMember("Market", mMarket.saveDoc(doc), allocator);

	Value robots(kObjectType);
	for (auto i = mRobots.begin(); i != mRobots.end(); ++i) {
		Robot* pRobot = *i;
		if (pRobot->position() != 0) {
			robots.AddMember(Value(pRobot->getKey().c_str(), allocator), pRobot->saveDoc(doc), allocator);
		}
	}
	doc.AddMember("Robots", robots, allocator);

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	doc.Accept(writer);

	std::string reststring = buffer.GetString();

	ofstream fs("./config/Record/Trader.json");

	if (!fs.good()) {
		Logger::Error() >> "<Trader.json> Save Failed!";
	}
	else {
		fs << buffer.GetString() << endl;
	}

	fs.close();

	return true;
}

void Trader::createAPIInstance() {
	char FLOW_FOLDER_TRADER[] = ".\\flow\\trader\\";
	char FLOW_FOLDER_MARKET[] = ".\\flow\\market\\";

	filesystem::remove_all(filesystem::path(FLOW_FOLDER_TRADER));
	filesystem::remove_all(filesystem::path(FLOW_FOLDER_MARKET));
	filesystem::create_directory(filesystem::path(FLOW_FOLDER_TRADER));
	filesystem::create_directory(filesystem::path(FLOW_FOLDER_MARKET));

	m_pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi(FLOW_FOLDER_TRADER);
	m_pMarketApi = CThostFtdcMdApi::CreateFtdcMdApi(FLOW_FOLDER_MARKET);

	m_pTraderSpi = new TraderSpi();
	m_pTraderApi->RegisterSpi((CThostFtdcTraderSpi*)m_pTraderSpi);			// 注册事件类
	m_pTraderApi->SubscribePublicTopic(THOST_TERT_RESTART);					// 注册公有流
	m_pTraderApi->SubscribePrivateTopic(THOST_TERT_RESTART);				// 注册私有流
	m_pTraderApi->RegisterFront(mAccounts[mInvestorID]->mFrontAddressTD[0]);// connect
	m_pTraderApi->Init();

	m_pMarketSpi = new MarketSpi();
	m_pMarketApi->RegisterSpi(m_pMarketSpi);								// 注册事件类
	m_pMarketApi->RegisterFront(mAccounts[mInvestorID]->mFrontAddressMD[0]);// connect
	m_pMarketApi->Init();
}