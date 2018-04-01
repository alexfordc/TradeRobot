#if !defined(TRADER_H)
#define TRADER_H

#include <vector>
#include "TimeUtil.h"
#include "Market.h"
#include "ctp/ThostFtdcTraderApi.h"
#include "ctp/ThostFtdcMdApi.h"
#include "MarketSpi.h"
#include "TraderSpi.h"

class Robot;
class Position;

enum TraderState {
	TS_SLEEP = 1,
	TS_INIT_API,
	TS_INIT_API_WAITING,
	TS_LOGIN,
	TS_LOGIN_WAITING,
	TS_PREPARED_WAITING,
	TS_TRADING,
	TS_LOGOUT,
	TS_LOGOUT_WAITING,
	TS_RECORD
};

struct Account
{
	TThostFtdcBrokerIDType mBrokerID;			// 经纪公司代码
	TThostFtdcInvestorIDType mInvestorID;		// 投资者代码
	TThostFtdcPasswordType mPassword;			// 用户密码

	std::vector<char*> mFrontAddressTD;
	std::vector<char*> mFrontAddressMD;
};

class Trader
{
public:
	static Trader& Instance();

	bool init(const std::string& investorID);

	~Trader();

	void update(long long dt);

	void addPosition(Position* pPos);

	void delPosition(Position* pPos);

	bool reqInsertOrder(CThostFtdcInputOrderField* req);

	bool reqCancelOrder(CThostFtdcInputOrderActionField* req);

	bool canSendRequest();

	bool reqQryTradingAccount();

	inline Market& getMarket() {
		return mMarket;
	}

	inline int getOrderRef() {
		return mOrderRef++;
	}

	inline double getMoney() const {
		return mMoney;
	}

	inline double getLotsRatio() const {
		return mLotsRatio;
	}

	inline int getParam1() const {
		return mParam1;
	}

	inline int getParam2() const {
		return mParam1;
	}

	bool canOpen();

	int posCount();

private:
	static Trader* m_pInstance;

	Trader();

	void parseContract();

	void parseRobot();

	void parseMisc();

	bool taskSleep();

	bool taskInitAPI();

	bool taskInitAPIWaiting();

	bool taskLogin();

	bool taskLoginWaiting();

	bool taskPreparedWaiting();
	
	bool taskTrading();

	bool taskLogout();

	bool taskLogoutWaiting();

	bool taskRecord();

	void createAPIInstance();

	Robot* createRobot(const char*, const char*, int);

	void onTimer();

	long long mTimeElapsed;

	CThostFtdcTraderApi* m_pTraderApi;
	CThostFtdcMdApi* m_pMarketApi;
	TraderSpi* m_pTraderSpi;
	MarketSpi* m_pMarketSpi;

	Market mMarket;

	std::vector<Robot*> mRobots;

	TraderState mState;

	char**	m_ppInstrumentID;

	int	mRequestID;
	int mOrderRef;
	int mPositionLimit;
	double mMoney;
	double mLotsRatio;
	int mParam1;
	int mParam2;

	std::map<std::string, Account*> mAccounts;
	std::string mInvestorID;
	boost::gregorian::date mLastTradingDay;
	boost::posix_time::ptime mRequestTime;
};

#endif
