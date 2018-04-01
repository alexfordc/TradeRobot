#include <boost/algorithm/string.hpp>
#include "Order.h"
#include "Global.h"
#include "Trader.h"
#include "Logger.h"

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace rapidjson;


Order::Order() {

}

Order::Order(const string& robot, const string& instrument, int volume, OrderTypeFlag type)
	:mRobot(robot), mInstrument(instrument), mVolume(volume), mType(type)
{
	mStatus = OS_WAITING;
	mDealVolume = 0;
	mAveragePrice = 0;

	mCreateTime = TimeUtil::Instance().getTime();
}

Order::Order(const string& line) {
	vector<string> words;
	boost::split(words, line, boost::is_any_of(","), token_compress_on);

	mTime = time_from_string(words[0]);
	mInstrument = words[1];
	mRobot = words[2];
	mType = (OrderTypeFlag)lexical_cast<int>(words[3]);
	mVolume = lexical_cast<int>(words[4]);
	mDealVolume = lexical_cast<int>(words[5]);
	mAveragePrice = lexical_cast<double>(words[6]);

	mCreateTime = TimeUtil::Instance().getTime();
}

Value Order::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("Time", Value(to_simple_string(mTime).c_str(), doc.GetAllocator()), doc.GetAllocator());
	v.AddMember("Instrument", Value(mInstrument.c_str(), doc.GetAllocator()), doc.GetAllocator());
	v.AddMember("Robot", Value(mInstrument.c_str(), doc.GetAllocator()), doc.GetAllocator());
	v.AddMember("OrderRef", mOrderRef, doc.GetAllocator());
	v.AddMember("Status", mStatus, doc.GetAllocator());
	v.AddMember("Type", mType, doc.GetAllocator());
	v.AddMember("Volume", mVolume, doc.GetAllocator());
	v.AddMember("DealVolume", mDealVolume, doc.GetAllocator());
	v.AddMember("AveragePrice", mAveragePrice, doc.GetAllocator());

	return v;
}

void Order::readDoc(const Value& v) {
	mTime = time_from_string(v["Time"].GetString());
	mInstrument = v["Instrument"].GetString();
	mRobot = v["Robot"].GetString();
	mOrderRef = v["OrderRef"].GetInt();
	mStatus = (OrderStatus)v["Status"].GetInt();
	mType = (OrderTypeFlag)v["Type"].GetInt();
	mVolume = v["Volume"].GetInt();
	mDealVolume = v["DealVolume"].GetInt();
	mAveragePrice = v["AveragePrice"].GetDouble();
}

string Order::toString() {
	string order = to_simple_string(mTime);
	order.push_back(',');

	order.append(mInstrument).push_back(',');
	order.append(mRobot).push_back(',');
	order.append(lexical_cast<string>(mType)).push_back(',');
	order.append(lexical_cast<string>(mVolume)).push_back(',');
	order.append(lexical_cast<string>(mDealVolume)).push_back(',');
	order.append(lexical_cast<string>(mAveragePrice));

	return order;
}

bool Order::insert(double price) {
	mOrderRef = Trader::Instance().getOrderRef();

	CThostFtdcInputOrderField req;

	memset(&req, 0, sizeof(CThostFtdcInputOrderField));

	strcpy(req.InstrumentID, mInstrument.c_str());
	strcpy(req.OrderRef, lexical_cast<std::string>(mOrderRef).c_str());

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	req.LimitPrice = price;
	req.Direction = (mType & OTF_BUY) ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
	if (mType & OTF_OPEN) {
		req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	}
	else {
		if (mType & OTF_TODAY) {
			req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
		}
		else {
			req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
		}
	}
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	//组合投机套保标志
	req.VolumeTotalOriginal = mVolume - mDealVolume;	//数量
	req.TimeCondition = THOST_FTDC_TC_GFD;				//有效期类型: 当日有效
	req.VolumeCondition = THOST_FTDC_VC_AV;				//成交量类型: 任何数量
	req.MinVolume = 1;									//最小成交量: 1
	req.ContingentCondition = THOST_FTDC_CC_Immediately;//触发条件: 立即
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;//强平原因: 非强平
	req.IsAutoSuspend = 0;								//自动挂起标志: 否
	req.UserForceClose = 0;								//用户强评标志: 否

	bool bSucc = Trader::Instance().reqInsertOrder(&req);
	if (bSucc) {
		mStatus = OS_INSERT;
		mTime = TimeUtil::Instance().getTime();
	}

	return bSucc;
}

bool Order::cancel() {
	if (mStatus != OS_TRADING) {
		return false;
	}

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(CThostFtdcInputOrderActionField));

	strcpy(req.InstrumentID, mInstrument.c_str());
	strcpy(req.OrderRef, lexical_cast<string>(mOrderRef).c_str());
	req.FrontID = Global::getFrontID();
	req.SessionID = Global::getSessionID();
	req.ActionFlag = THOST_FTDC_AF_Delete;

	bool bSucc = Trader::Instance().reqCancelOrder(&req);
	if (bSucc) {
		mStatus = OS_CANCELING;
		mTime = TimeUtil::Instance().getTime();
	}

	return bSucc;
}

void Order::onMsg(const OrderMsg* pMsg) {
	switch (pMsg->Type)
	{
	case OrderMsg::OMT_INSERT_REJECTED:
		if (mStatus == OS_INSERT) {
			mStatus = OS_WAITING;
		}
		break;
	case OrderMsg::OMT_CANCEL_REJECTED:
		if (mStatus == OS_CANCELING) {
			mStatus = OS_TRADING;
		}
		break;
	case OrderMsg::OMT_CANCELED:
		if (mStatus == OS_CANCELING) {
			mStatus = OS_WAITING;
		}
		break;
	case OrderMsg::OMT_TRADING:
		if (mStatus == OS_INSERT) {
			mStatus = OS_TRADING;
		}
		break;
	case OrderMsg::OMT_TRADE:
		rspTrade(pMsg->Volume, pMsg->Price);
		break;
	case OrderMsg::OMT_NOT_TRADING:
		mStatus = OS_WAITING;
		break;
	case OrderMsg::OMT_ALL_TRADED:
		mStatus = OS_FINISH;
		break;
	default:
		break;
	}

	mTime = TimeUtil::Instance().getTime();
}

void Order::rspTrade(int volume, double price) {
	mAveragePrice = ((mAveragePrice * mDealVolume) + (price * (volume - mDealVolume))) / volume;
	mDealVolume = volume;

	if (mDealVolume == mVolume && mStatus == OS_TRADING) {
		mStatus = OS_FINISH;
	}

	mTime = TimeUtil::Instance().getTime();
}

bool Order::isWaiting() {
	return mStatus == OS_WAITING;
}

bool Order::isOvertime() {
	return OS_TRADING == mStatus && TimeUtil::Instance().getTime() - mTime > seconds(6);
}

bool Order::isFinish() {
	return mStatus == OS_FINISH;
}

bool Order::isDead() {
	return TimeUtil::Instance().getTime() - mCreateTime > seconds(600);
}