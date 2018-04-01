#include "TraderSpi.h"
#include "Logger.h"
#include "Global.h"

using namespace std;
using namespace boost;

TraderSpi::TraderSpi()
{	
}
	
	
TraderSpi::~TraderSpi()
{	
}
	
void TraderSpi::OnFrontConnected()
{
	Logger::Info() >> "Front Trader Server Connected!";
	Global::onConnectedTD();
}
	
void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (bIsLast && !isErrorRspInfo(pRspInfo))
	{
		Global::onUserLoginTD(pRspUserLogin);
	}
}

void TraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (bIsLast && !isErrorRspInfo(pRspInfo))
	{
		Global::onUsrLogoutTD();
	}

}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !isErrorRspInfo(pRspInfo))
	{
		Global::onPreparedTD();
		Logger::Info() >> "Trader Ready!";
	}
}

void TraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !isErrorRspInfo(pRspInfo)) {

	}
}
	
void TraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !isErrorRspInfo(pRspInfo))
	{
		Logger::Info()
			<< "	TradingDay:	" << pTradingAccount->TradingDay
			<< "	Balance:	" << pTradingAccount->Balance
			<< "	CurrMargin:	" >> pTradingAccount->CurrMargin;
	}
}
		
//void QTraderSpi::reqQryInvestorPosition()
//{
	//	cout << ">>> " << "查询持仓..." << endl;
//	CThostFtdcQryInvestorPositionField req;
//	memset(&req, 0, sizeof(req));
//	strcpy_s(req.BrokerID, m_pGlobalDataExchange->getBrokerID());
//	strcpy_s(req.InvestorID, m_pGlobalDataExchange->getInvestorID());
//	strcpy_s(req.InstrumentID, "AL1609");
//	int iResult = m_pTraderApi->ReqQryInvestorPosition(&req, m_pGlobalDataExchange->sendRequestID());
//}
		
///请求查询投资者持仓响应
void TraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !isErrorRspInfo(pRspInfo))
	{
		cout << pInvestorPosition->Position << endl;
		///合约保证金以及手续费查询请求
		//for (int i = 0; i < 20; i++)
		//{
			//strcpy_s(INSTRUMENT_ID, InstrumentID_n[3]);
			//ReqQryInstrumentMarginRate();
			//ReqQryInstrumentCommissionRate();
			//Sleep(1000);
		//}
	}
	//END
}
		
/////请求查询合约保证金率
//void QTraderSpi::ReqQryInstrumentMarginRate()
//{
//	//cerr << "--->>> 查询合约保证金率"<< endl;
//	CThostFtdcQryInstrumentMarginRateField req;
//	memset(&req, 0, sizeof(req));
//
//	///经纪公司代码
//	strcpy_s(req.BrokerID, Global::getBrokerID());
//	///投资者代码
//	strcpy_s(req.InvestorID, Global::getInvestorID());
//	///合约代码
//	strcpy_s(req.InstrumentID, "AL1609");
//	//////投机套保标志
//	req.HedgeFlag = THOST_FTDC_HF_Speculation;	//投机
//
//	int iResult = m_pTraderApi->ReqQryInstrumentMarginRate(&req, Global::sendRequestID());
//	//cerr << "--->>> 查询合约保证金率: " << ((iResult == 0) ? " 成功" : " 失败") << endl;
//}
//		
/////请求查询合约手续费率
//void QTraderSpi::ReqQryInstrumentCommissionRate()
//{
//	//cerr << "--->>> 查询合约手续费率"<< endl;
//	CThostFtdcQryInstrumentCommissionRateField req;
//	memset(&req, 0, sizeof(req));
//
//	///经纪公司代码
//	strcpy_s(req.BrokerID, Global::getBrokerID());
//	///投资者代码
//	strcpy_s(req.InvestorID, Global::getInvestorID());
//	///合约代码
//	strcpy_s(req.InstrumentID, "AL1609");
//
//	int iResult = m_pTraderApi->ReqQryInstrumentCommissionRate(&req, Global::sendRequestID());
//	//cerr << "--->>> 查询合约手续费率: " << ((iResult == 0) ? " 成功" : " 失败") << endl;
//}
			
// 报单后，如不能通过THOST校验，THOST拒绝报单，返回OnRspOrderInsert（含错误编码）。如果校验通过，THOST接收报单，转发给交易所
// 交易所收到报单后，通过校验。用户会收到OnRtnOrder、OnRtnTrade。
// 如果交易所认为报单错误，用户就会收到OnErrRtnOrder
void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	isErrorRspInfo(pRspInfo);
}
			
void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	isErrorRspInfo(pRspInfo);	
}
				
	///报单通知
void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	switch (pOrder->OrderSubmitStatus)
	{
		case THOST_FTDC_OSS_InsertSubmitted:
			break;
		case THOST_FTDC_OSS_CancelSubmitted:
			break;
		case THOST_FTDC_OSS_Accepted:
			break;
		case THOST_FTDC_OSS_InsertRejected:
			Logger::Info() << "!!! Insert Rejected:" >> pOrder->OrderStatus;
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_INSERT_REJECTED);
			break;
		case THOST_FTDC_OSS_CancelRejected:
			Logger::Info() << "!!! Cancel Rejected:" >> pOrder->OrderStatus;
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_CANCEL_REJECTED);
			break;
		default:
			break;
	}

	switch (pOrder->OrderStatus)
	{
		case THOST_FTDC_OST_AllTraded:
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_ALL_TRADED);
			break;
		case THOST_FTDC_OST_PartTradedQueueing:
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_TRADING);
			break;
		case THOST_FTDC_OST_PartTradedNotQueueing:
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_NOT_TRADING);
			break;
		case THOST_FTDC_OST_NoTradeQueueing:
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_TRADING);
			break;
		case THOST_FTDC_OST_NoTradeNotQueueing:
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_NOT_TRADING);
			break;
		case THOST_FTDC_OST_Canceled:
			Global::pushOrderMsg(pOrder->InstrumentID, pOrder->OrderRef, OrderMsg::OMT_CANCELED);
			break;
		case THOST_FTDC_OST_Unknown:
			break;
		default:
			Logger::Info() << "!!! Unknow OrderStatus:" >> pOrder->OrderStatus;
			break;
	}
}
	
///成交通知
void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	Global::pushOrderMsg(pTrade->InstrumentID, pTrade->OrderRef, OrderMsg::OMT_TRADE, pTrade->Volume, pTrade->Price);
}

void TraderSpi::OnFrontDisconnected(int nReason)
{
	switch (nReason)
	{
	case 0x1001:
		Logger::Info() >> "Front Trader Server Disconnected! Reason: 网络读失败";
		break;
	case 0x1002:
		Logger::Info() >>"Front Trader Server Disconnected! Reason: 网络写失败";
		break;
	case 0x2001:
		Logger::Info() >> "Front Trader Server Disconnected! Reason: 接收心跳超时";
		break;
	case 0x2002:
		Logger::Info() >>"Front Trader Server Disconnected! Reason: 发送心跳失败";
		break;
	case 0x2003:
		Logger::Info() >>"Front Trader Server Disconnected! Reason: 收到错误报文";
		break;
	default:
		Logger::Info() <<"Front Trader Server Disconnected! Reason: ">> nReason;
		break;
	}

	Global::onDisconnectedTD();
}

void TraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	Logger::Error() << "Front Trader Server HeartBeatWarning! TimeLapse: " >> nTimeLapse;
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	isErrorRspInfo(pRspInfo);
}

bool TraderSpi::isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = pRspInfo != nullptr && pRspInfo->ErrorID != 0;
	if (bResult) {
		Logger::Error() << "TD! Msg:" << pRspInfo->ErrorMsg << " ID:" >> pRspInfo->ErrorID;
	}

	return bResult;
}
					
///请求查询合约保证金率响应
void TraderSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	double bzj_b = 100 * (pInstrumentMarginRate->LongMarginRatioByMoney);
}

///请求查询合约手续费率响应
void TraderSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	double sxf_w = 100 * 100 * (pInstrumentCommissionRate->OpenRatioByMoney);
}
			