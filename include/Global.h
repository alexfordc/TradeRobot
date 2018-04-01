#if !defined(GLOBAL_BUF_H)
#define GLOBAL_BUF_H

#include <string>
#include <queue>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "ctp/ThostFtdcUserApiStruct.h"
#include "TickData.h"
#include "OrderMsg.h"


namespace Global
{
	enum TraderStatus
	{
		TS_SLEEPING,
		TS_CONNECTED,
		TS_LOGIN,
		TS_PREPARED,
		TS_LOGOUT,
		TS_DISCONNECTED
	};

	enum MarketStatus
	{
		MS_SLEEPING,
		MS_CONNECTED,
		MS_LOGIN,
		MS_PREPARED,
		MS_LOGOUT,
		MS_DISCONNECTED
	};

	void reset();

	void onConnectedTD();

	void onConnectedMD();

	void onUserLoginTD(CThostFtdcRspUserLoginField* pRspUserLoginField);

	void onUserLoginMD(CThostFtdcRspUserLoginField* pRspUserLoginField);

	void onPreparedTD();

	void onPreparedMD();

	void onUsrLogoutTD();

	void onUsrLogoutMD();

	void onDisconnectedTD();

	void onDisconnectedMD();

	int getFrontID();

	int getSessionID();

	const boost::gregorian::date& getTradingDay();

	TraderStatus getStatusTD();

	MarketStatus getStatusMD();

	const std::string& getLoginTime();

	void pushTick(CThostFtdcDepthMarketDataField* pTick);

	bool isTickEmpty();

	TickData* popTick();

	void pushOrderMsg(const std::string& instrumentId, const std::string& orderRef, OrderMsg::OrderMsgType type, int volume=0, double price=0.0);

	bool isMsgEmpty();

	OrderMsg* popMsg();
}

#endif
