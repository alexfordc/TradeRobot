#if !defined(CONTRACT_H)
#define CONTRACT_H

#include <vector>
#include <list>
#include "rapidjson/document.h"
#include "Chart.h"
#include "TickData.h"
#include "Order.h"
#include "OrderMsg.h"

class Robot;

class Contract
{
public:
	Contract(const std::string& name, const std::string& timeKey, 
		double tickPrice, bool isShangHai, int quantity, double margin);

	~Contract();

	void addRobot(Robot*);

	void onTick(const TickData* pTick);

	void onRobotTick();

	inline const std::string& getName() const {
		return mName;
	}

	//Contract Trading Time Set
	inline const std::string& getTimeKey() const {
		return mTimeKey;
	}

	//Price Smallest Change
	inline double getTickPrice() const {
		return mTickPrice;
	}

	inline bool isShangHai() const {
		return mIsShangHai;
	}

	inline int getQuantity() const {
		return mQuantity;
	}

	inline double getMargin() const{
		return mMargin;
	}

	void save();

	void read();

	rapidjson::Value saveDoc(rapidjson::Document& doc);

	void readDoc(const rapidjson::Value& v);

	inline const Chart* getChart(Chart::PeriodType pt) const {
		auto it = mCharts.find(pt);
		if (it != mCharts.end()) {
			return it->second;
		}

		return nullptr;
	}

	void buy(Robot* pRobot, int volume);

	void sell(Robot* pRobot, int volume);

	void buyToClose(Robot* pRobot, int volume);

	void sellToClose(Robot* pRobot, int volume);

	bool rspOrderMsg(const OrderMsg* pMsg);

	void insertOrder();

	void cancelOrder();

	void removeOrder();

	void resetTradingDay();

	Robot* getRobotByName(const std::string& name);

	inline std::list<Order*>& getFinishOrders() {
		return mFinishOrders;
	}

	bool isUpperLimited() const {
		double price = mLastTick.AskPrice + mTickPrice;
		if (price > mLastTick.UpperLimitPrice) {
			return true;
		}
		return false;
	}

	bool isLowerLimited() const {
		double price = mLastTick.BidPrice - mTickPrice;
		if (price < mLastTick.LowerLimitPrice) {
			return true;
		}
		return false;
	}

	bool hasPosition() const;

private:
	inline double getBuyPrice() {
		double price = mLastTick.AskPrice + mTickPrice;
		if (price > mLastTick.UpperLimitPrice) {
			price = mLastTick.UpperLimitPrice;
		}

		return price;
	}

	inline double getSellPrice() {
		double price = mLastTick.BidPrice - mTickPrice;
		if (price < mLastTick.LowerLimitPrice) {
			price = mLastTick.LowerLimitPrice;
		}

		return price;
	}

	std::string mName;
	bool mIsShangHai;
	std::map<Chart::PeriodType, Chart*> mCharts;
	std::vector<Robot*> mRobots;
	std::string mTimeKey;
	double mTickPrice;
	TickData mLastTick;
	int mQuantity;
	double mMargin;

	int mTdBuyPosition;
	int mTdSellPosition;

	std::list<Order*> mOrders;
	std::list<Order*> mFinishOrders;
	bool tickPrepared;
};

#endif