#include "MarketSpi.h"
#include "Logger.h"
#include "Global.h"


using namespace std;
using namespace boost;

MarketSpi::MarketSpi() {
}

MarketSpi::~MarketSpi() {
}

void MarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	isErrorRspInfo(pRspInfo);
}

void MarketSpi::OnFrontDisconnected(int nReason)
{
	switch (nReason)
	{
	case 0x1001:
		Logger::Info() >> "Front Market Server Disconnected! Reason: �����ʧ��";
		break;
	case 0x1002:
		Logger::Info() >> "Front Market Server Disconnected! Reason: ����дʧ��";
		break;
	case 0x2001:
		Logger::Info() >> "Front Market Server Disconnected! Reason: ����������ʱ";
		break;
	case 0x2002:
		Logger::Info() >> "Front Market Server Disconnected! Reason: ��������ʧ��";
		break;
	case 0x2003:
		Logger::Info() >> "Front Market Server Disconnected! Reason: �յ�������";
		break;
	default:
		Logger::Info() << "Front Market Server Disconnected! Reason: " >> nReason;
		break;
	}

	Global::onDisconnectedMD();
}

void MarketSpi::OnHeartBeatWarning(int nTimeLapse)
{
	Logger::Error() << "Front Market Server HeartBeatWarning! TimeLapse: " >> nTimeLapse;
}

void MarketSpi::OnFrontConnected()
{
	Logger::Info() >> "Front Market Server Connected!";
	Global::onConnectedMD();
}

void MarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!isErrorRspInfo(pRspInfo) && bIsLast)
	{
		Global::onUserLoginMD(pRspUserLogin);
	}
}

void MarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (!isErrorRspInfo(pRspInfo) && bIsLast)
	{
		Global::onUsrLogoutMD();
	}
}

void MarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!isErrorRspInfo(pRspInfo)) {
		if (bIsLast) {
			Global::onPreparedMD();
			Logger::Info() >> "Market Data Ready!";
		}
	}
}

void MarketSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

void MarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	Global::pushTick(pDepthMarketData);
}

bool MarketSpi::isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool bResult = pRspInfo!=nullptr && pRspInfo->ErrorID != 0;
	if (bResult) {
		Logger::Error() << "MD! Msg:" << pRspInfo->ErrorMsg << " ID:" >> pRspInfo->ErrorID;
	}
		
	return bResult;
}
