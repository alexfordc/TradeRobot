#if !defined(ORDER_H)
#define ORDER_H

#include <string>
#include "rapidjson/document.h"
#include "TimeUtil.h"
#include "OrderMsg.h"


class Order {
public:
	enum OrderStatus {
		OS_WAITING = 1,	//等待下单
		OS_INSERT,		//发出报单
		OS_TRADING,		//交易中
		OS_CANCELING,	//撤单中
		OS_FINISH		//交易完成
	};

	enum OrderTypeFlag {
		OTF_BUY = 1,
		OTF_SELL = 2,
		OTF_OPEN = 4,
		OTF_CLOSE = 8,
		OTF_TODAY = 16,
		OTF_BUY_OPEN = 5,
		OTF_SELL_OPEN = 6,
		OTF_BUY_CLOSE = 9,
		OTF_SELL_CLOSE = 10,
		OTF_BUY_CLOSE_TODAY = 25,
		OTF_SELL_CLOSE_TODAY = 26
	};

	Order();

	Order(const std::string& robot, const std::string& instrument, int volume, OrderTypeFlag type);

	Order(const std::string& line);

	rapidjson::Value saveDoc(rapidjson::Document& doc);

	void readDoc(const rapidjson::Value& v);

	std::string toString();

	bool insert(double price);

	bool cancel();

	void onMsg(const OrderMsg* pMsg);

	inline bool hasFlag(OrderTypeFlag flag) const {
		return (mType & flag) != 0;
	}

	inline int getVolume() const {
		return mVolume;
	}

	inline int geDealVolume() const {
		return mDealVolume;
	}

	inline int getUnDealVolume() const {
		return mVolume - mDealVolume;
	}

	inline const std::string& getRobot() const {
		return mRobot;
	}

	inline const int getOrderRef() const {
		return mOrderRef;
	}

	inline OrderTypeFlag getType() const {
		return mType;
	}

	bool isWaiting();

	bool isFinish();

	bool isOvertime();

	bool isDead();

private:
	void rspTrade(int volume, double price);

	int mOrderRef;
	std::string mInstrument;
	std::string mRobot;
	OrderStatus mStatus;
	OrderTypeFlag mType;
	int mVolume;
	int mDealVolume;
	double mAveragePrice;
	boost::posix_time::ptime mTime;
	boost::posix_time::ptime mCreateTime;
};

#endif