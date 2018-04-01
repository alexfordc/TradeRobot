#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include "Global.h"

using namespace std;
using namespace boost;

namespace Global {
	boost::mutex MutexTD;
	boost::mutex MutexMD;

	std::queue<TickData*> TickQueue;
	std::queue<OrderMsg*> OrderMsgQueue;
	string LoginTime;
	int FrontID	= 0;
	int SessionID	= 0;
	//int OrderRef	= TimeUtil::Instance().getTime().time_of_day().total_seconds();
	gregorian::date TradingDay = gregorian::date(gregorian::min_date_time);
	TraderStatus StatusTD = TS_SLEEPING;
	MarketStatus StatusMD = MS_SLEEPING;
	

	void reset() {
		FrontID = 0;
		SessionID = 0;
		//OrderRef = TimeUtil::Instance().getTime().time_of_day().total_seconds();
		StatusTD = TS_SLEEPING;
		StatusMD = MS_SLEEPING;

		while (!TickQueue.empty()) {
			TickData* pTick = TickQueue.front();
			TickQueue.pop();
			delete pTick;
		}

		while (!OrderMsgQueue.empty()) {
			OrderMsg* pMsg = OrderMsgQueue.front();
			OrderMsgQueue.pop();
			delete pMsg;
		}
	}

	void onConnectedTD() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		StatusTD = TS_CONNECTED;
	}

	void onConnectedMD() {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		StatusMD = MS_CONNECTED;
	}

	void onUserLoginTD(CThostFtdcRspUserLoginField* pRspUserLoginField) {
		boost::lock_guard<boost::mutex> lock(MutexTD);

		LoginTime = pRspUserLoginField->LoginTime;
		FrontID = pRspUserLoginField->FrontID;
		SessionID = pRspUserLoginField->SessionID;
		//OrderRef = atoi(pRspUserLoginField->MaxOrderRef);
		StatusTD = TS_LOGIN;
		TradingDay = boost::gregorian::from_undelimited_string(pRspUserLoginField->TradingDay);
	}

	void onUserLoginMD(CThostFtdcRspUserLoginField* pRspUserLoginField) {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		StatusMD = MS_LOGIN;
	}

	void onPreparedTD() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		StatusTD = TS_PREPARED;
	}

	void onPreparedMD() {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		StatusMD = MS_PREPARED;
	}

	void onUsrLogoutTD() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		StatusTD = TS_LOGOUT;
	}

	void onUsrLogoutMD() {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		StatusMD = MS_LOGOUT;
	}

	void onDisconnectedTD() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		StatusTD = TS_DISCONNECTED;
	}

	void onDisconnectedMD() {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		StatusMD = MS_DISCONNECTED;
	}

	TraderStatus getStatusTD() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		return StatusTD;
	}

	MarketStatus getStatusMD() {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		return StatusMD;
	}

	const string& getLoginTime() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		return LoginTime;
	}

	//int getOrderRef() {
	//	boost::lock_guard<boost::mutex> lock(MutexTD);
	//	return OrderRef++;
	//}

	int getFrontID() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		return FrontID;
	}

	int getSessionID() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		return SessionID;
	}

	const boost::gregorian::date& getTradingDay() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		return TradingDay;
	}

	void pushTick(CThostFtdcDepthMarketDataField* pTick) {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		TickQueue.push(new TickData(pTick));
	}

	bool isTickEmpty() {
		boost::lock_guard<boost::mutex> lock(MutexMD);
		return TickQueue.empty();
	}

	TickData* popTick() {
		boost::lock_guard<boost::mutex> lock(MutexMD);

		if (TickQueue.empty()) {
			return nullptr;
		}

		TickData* pTick = TickQueue.front();
		TickQueue.pop();

		return pTick;
	}

	void pushOrderMsg(const string& instrumentId, const string& orderRef, OrderMsg::OrderMsgType type, int volume, double price) {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		OrderMsgQueue.push(new OrderMsg(instrumentId, lexical_cast<int>(trim_left_copy(orderRef)), type, volume, price));
	}

	bool isMsgEmpty() {
		boost::lock_guard<boost::mutex> lock(MutexTD);
		return OrderMsgQueue.empty();
	}

	OrderMsg* popMsg() {
		boost::lock_guard<boost::mutex> lock(MutexTD);

		if (OrderMsgQueue.empty()) {
			return nullptr;
		}

		OrderMsg* pMsg = OrderMsgQueue.front();
		OrderMsgQueue.pop();

		return pMsg;
	}
}