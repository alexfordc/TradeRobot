#include <boost/lexical_cast.hpp>
#include "Contract.h"
#include "Robot.h"
#include "TimeUtil.h"
#include "Market.h"
#include "Logger.h"


using namespace boost::posix_time;
using namespace std;
using namespace boost;
using namespace rapidjson;

Contract::Contract(const string& name, const string& timeKey, 
	double tickPrice, bool isShangHai, int quantity, double margin):
	mName(name), mTimeKey(timeKey), mTickPrice(tickPrice), mIsShangHai(isShangHai),
	mQuantity(quantity), mMargin(margin)
{
	//mCharts[Chart::PT_PERIOD_M5] = new Chart(mName, Chart::PT_PERIOD_M5);
	mCharts[Chart::PT_PERIOD_M15] = new Chart(mName, Chart::PT_PERIOD_M15);
	mCharts[Chart::PT_PERIOD_D1] = new Chart(mName, Chart::PT_PERIOD_D1);
	tickPrepared = false;
}

Contract::~Contract()
{
}

bool Contract::hasPosition() const {
	for (auto it = mRobots.begin(); it != mRobots.end(); ++it) {
		if ((*it)->position() != 0) {
			return true;
		}
	}

	return false;
}

void Contract::addRobot(Robot* pRobot) {
	for (auto it = mRobots.begin(); it != mRobots.end(); ++it) {
		if (*it == pRobot) {
			return ;
		}
	}

	mRobots.push_back(pRobot);
	pRobot->setContract(this);
}

void Contract::onTick(const TickData* pTick) {
	if (mLastTick.Time <= pTick->Time) {
		mLastTick.copy(*pTick);
		tickPrepared = true;
	}
	else {
		Logger::Error() << "Disorder Tick! Contract: " << mName << ", " >> pTick->InstrumentID;
		return ;
	}

	vector<const Chart*> newBarCharts;
	for (auto it = mCharts.begin(); it != mCharts.end(); ++it) {
		if (it->second->mergeTick(pTick)) {
			newBarCharts.push_back(it->second);
		}
	}

	for (auto it = newBarCharts.begin(); it != newBarCharts.end(); ++it) {
		for (auto itr = mRobots.begin(); itr != mRobots.end(); ++itr) {
			Robot* pRobot = *itr;
			pRobot->onBarStart(*it);
		}
	}
	newBarCharts.clear();
}

void Contract::onRobotTick() {
	if (!tickPrepared) {
		return;
	}
	for (auto itr = mRobots.begin(); itr != mRobots.end(); ++itr) {
		Robot* pRobot = *itr;
		pRobot->onTick(mLastTick.Time, mLastTick.Price, mLastTick.AskPrice, mLastTick.AskVolume, mLastTick.BidPrice, mLastTick.BidVolume);
	}
	tickPrepared = false;
}

void Contract::save() {
	for (auto it = mCharts.begin(); it != mCharts.end(); ++it) {
		it->second->save();
	}

	for (auto it = mOrders.begin(); it != mOrders.end(); ) {
		Order* pOrder = *it;
		Logger::Info() >> "!!!!!!!!!!!!!!! Not Finish Order !!!!!!!!!!!!!!";
		Logger::Info() >> pOrder->toString();
		Robot* pRobot = getRobotByName(pOrder->getRobot());
		if (pRobot != nullptr) {
			pRobot->clearPosition(pOrder->getUnDealVolume());
		}
		it = mOrders.erase(it);
		mFinishOrders.push_back(pOrder);
	}
}

void Contract::read() {
	for (auto it = mCharts.begin(); it != mCharts.end(); ++it) {
		it->second->read();
	}
}

Value Contract::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("TdBuyPosition", mTdBuyPosition, doc.GetAllocator());
	v.AddMember("TdSellPosition", mTdSellPosition, doc.GetAllocator());

	Value cv(kObjectType);
	for (auto it = mCharts.begin(); it != mCharts.end(); ++it) {
		switch (it->first)
		{
		case Chart::PT_PERIOD_M5:
			cv.AddMember("M5", it->second->saveDoc(doc), doc.GetAllocator());
			break;
		case Chart::PT_PERIOD_M15:
			cv.AddMember("M15", it->second->saveDoc(doc), doc.GetAllocator());
			break;
		case Chart::PT_PERIOD_D1:
			cv.AddMember("D1", it->second->saveDoc(doc), doc.GetAllocator());
			break;
		default:
			break;
		}
	}
	v.AddMember("Chart", cv, doc.GetAllocator());

	return v;
}

Robot* Contract::getRobotByName(const std::string& name) {
	for (auto it = mRobots.begin(); it != mRobots.end(); ++it) {
		if ((*it)->getName().compare(name) == 0) {
			return *it;
		}
	}

	return nullptr;
}

void Contract::readDoc(const Value& v) {
	mTdBuyPosition = v["TdBuyPosition"].GetInt();
	mTdSellPosition = v["TdSellPosition"].GetInt();

	const Value& cv = v["Chart"];
	for (auto it = mCharts.begin(); it != mCharts.end(); ++it) {
		switch (it->first)
		{
		case Chart::PT_PERIOD_M5:
			if (cv.HasMember("M5")) {
				it->second->readDoc(cv["M5"]);
			}
			break;
		case Chart::PT_PERIOD_M15:
			if (cv.HasMember("M15")) {
				it->second->readDoc(cv["M15"]);
			}
			break;
		case Chart::PT_PERIOD_D1:
			if (cv.HasMember("D1")) {
				it->second->readDoc(cv["D1"]);
			}
			break;
		default:
			break;
		}
	}
}

void Contract::buy(Robot* pRobot, int volume) {
	Logger::Info() << mName << ", Robot:" << pRobot->getName() << ", Buy:" >> volume;
	Order* pOrder = new Order(pRobot->getName(), mName, volume, Order::OTF_BUY_OPEN);
	mOrders.push_back(pOrder);
	mTdBuyPosition += volume;
}

void Contract::sell(Robot* pRobot, int volume) {
	Logger::Info() << mName << ", Robot:" << pRobot->getName() << ", Sell:" >> volume;
	Order* pOrder = new Order(pRobot->getName(), mName, volume, Order::OTF_SELL_OPEN);
	mOrders.push_back(pOrder);
	mTdSellPosition += volume;
}

void Contract::buyToClose(Robot* pRobot, int volume) {
	Logger::Info() << mName << ", Robot:" << pRobot->getName() << ", Buy to close:" >> volume;

	if (mIsShangHai) {
		if (mTdSellPosition >= volume) {
			Order* pOrder = new Order(pRobot->getName(), mName, volume, Order::OTF_BUY_CLOSE_TODAY);
			mOrders.push_back(pOrder);
		}
		else {
			Order* pOrder = nullptr;
			if (mTdSellPosition > 0)
			{
				pOrder = new Order(pRobot->getName(), mName, mTdSellPosition, Order::OTF_BUY_CLOSE_TODAY);
				mOrders.push_back(pOrder);
			}
			pOrder = new Order(pRobot->getName(), mName, volume - mTdSellPosition, Order::OTF_BUY_CLOSE);
			mOrders.push_back(pOrder);
		}
	}
	else {
		Order* pOrder = new Order(pRobot->getName(), mName, volume, Order::OTF_BUY_CLOSE);
		mOrders.push_back(pOrder);
	}
	mTdSellPosition -= volume;
	if (mTdSellPosition < 0) {
		mTdSellPosition = 0;
	}
}

void Contract::sellToClose(Robot* pRobot, int volume) {
	Logger::Info() << mName << ", Robot:" << pRobot->getName() << ", Sell to close:" >> volume;

	if (mIsShangHai) {
		if (mTdBuyPosition >= volume) {
			Order* pOrder = new Order(pRobot->getName(), mName, volume, Order::OTF_SELL_CLOSE_TODAY);
			mOrders.push_back(pOrder);
		}
		else {
			Order* pOrder = nullptr;
			if (mTdBuyPosition > 0) {
				pOrder = new Order(pRobot->getName(), mName, mTdBuyPosition, Order::OTF_SELL_CLOSE_TODAY);
				mOrders.push_back(pOrder);
			}
			pOrder = new Order(pRobot->getName(), mName, volume - mTdBuyPosition, Order::OTF_SELL_CLOSE);
			mOrders.push_back(pOrder);
		}
	}
	else {
		Order* pOrder = new Order(pRobot->getName(), mName, volume, Order::OTF_SELL_CLOSE);
		mOrders.push_back(pOrder);
	}
	mTdBuyPosition -= volume;
	if (mTdBuyPosition < 0) {
		mTdBuyPosition = 0;
	}
}

bool Contract::rspOrderMsg(const OrderMsg* pMsg) {
	for (auto it = mOrders.begin(); it != mOrders.end(); ++it) {
		Order* pOrder = *it;
		if (pMsg->OrderRef == pOrder->getOrderRef()) {
			pOrder->onMsg(pMsg);
			return true;
		}
	}

	return false;
}

void Contract::insertOrder() {
	for (auto it = mOrders.begin(); it != mOrders.end(); ++it) {
		Order* pOrder = *it;

		if (pOrder->isWaiting()) {
			if (pOrder->getType() & Order::OTF_BUY) {
				double price = getBuyPrice();
				if (!pOrder->insert(price)) {
					Logger::Error() << "Insert Order Failed! Contract:" << mName << ", Robot:" >> pOrder->getRobot();
				}
				Logger::Info() << mName << ", Price:" << price << ", Ref:" >> pOrder->getOrderRef();
			}
			else if (pOrder->getType() & Order::OTF_SELL) {
				double price = getSellPrice();
				if (!pOrder->insert(price)) {
					Logger::Error() << "Insert Order Failed! Contract:" << mName << ", Robot:" >> pOrder->getRobot();
				}
				Logger::Info() << mName << ", Price:" << price << ", Ref:" >> pOrder->getOrderRef();
			}
			break;
		}
	}
}

void Contract::cancelOrder() {
	for (auto it = mOrders.begin(); it != mOrders.end(); ++it) {
		Order* pOrder = *it;
		if (pOrder->isOvertime()) {
			if (!(isUpperLimited() && pOrder->hasFlag(Order::OTF_BUY)) &&
				!(isLowerLimited() && pOrder->hasFlag(Order::OTF_SELL))) {
				Logger::Info() << "Cancel, Ref:" >> pOrder->getOrderRef();
				pOrder->cancel();
				break;
			}
		}
	}
}

void Contract::removeOrder() {
	for (auto it = mOrders.begin(); it != mOrders.end();) {
		Order* pOrder = *it;
		if (pOrder->isFinish()) {
			Logger::Info() << "Finish" << ", Ref:" >> pOrder->getOrderRef();
			it = mOrders.erase(it);
			mFinishOrders.push_back(pOrder);
		}
		else if (pOrder->isDead() && !pOrder->hasFlag(Order::OTF_CLOSE) && pOrder->geDealVolume() == 0) {
			Logger::Info() >> "!!!!!!!!!!!!!!! Order Dead !!!!!!!!!!!!!!";
			Logger::Info() >> pOrder->toString();
			Robot* pRobot = getRobotByName(pOrder->getRobot());
			if (pRobot != nullptr) {
				pRobot->clearPosition(pOrder->getUnDealVolume());
			}
			it = mOrders.erase(it);
			mFinishOrders.push_back(pOrder);
		}
		else {
			++it;
		}
	}
}

void Contract::resetTradingDay() {
	mTdBuyPosition = 0;
	mTdSellPosition = 0;

	for (auto it = mCharts.begin(); it != mCharts.end(); ++it) {
		it->second->resetDayVolume();
	}
}