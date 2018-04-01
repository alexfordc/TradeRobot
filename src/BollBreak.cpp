#include "BollBreak.h"
#include <boost/filesystem.hpp>
#include "TimeUtil.h"
#include "Logger.h"
#include "Trader.h"

using namespace std;
using namespace rapidjson;
using namespace boost;
using namespace boost::posix_time;


BollBreak::BollBreak(const Value& params, int id) : Robot("BollBreak", id)
{
	N = params["N"].GetInt();
	Lots = 0;
	mStopPrice = 0.0;
	K = 1.2;
	S = 1000000.0;
	mInitialized = false;
	m_pLastBar = nullptr;
}

BollBreak::~BollBreak()
{
}

void BollBreak::readDoc(const Value& v) {
	setPosition(v["Position"].GetInt());
	mStopPrice = v["StopPrice"].GetDouble();
}

Value BollBreak::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("Position", position(), doc.GetAllocator());
	v.AddMember("StopPrice", mStopPrice, doc.GetAllocator());

	return v;
}

// Order: onBarStart -> onTick -> onTimer
void BollBreak::onTick(const ptime& time, double price,
	double askPrice, int askVolume, double bidPrice, int bidVolume) {
	if (!mInitialized || Lots == 0) {
		return;
	}

	if (position() > 0 && (price < mStopPrice || m_pLastBar->High < upper)
		&& m_pLastBar->Low <= m_pLastBar->UpperLimitPrice - getTickPrice()) {
		sellToClose(position());
	}

	if (position() < 0 && (price > mStopPrice || m_pLastBar->Low > lower)
		&& m_pLastBar->High >= m_pLastBar->LowerLimitPrice + getTickPrice()) {
		buyToClose(-position());
	}

	if (m_pLastBar->Low > upper && price > HH && position() <= 0) {
		if (position() < 0) {
			buyToClose(-position());
		}
		if (Lots > 0 && Trader::Instance().canOpen() && !isUpperLimited()) {
			buy(Lots);
			mStopPrice = price - S;
		}
	}

	if (m_pLastBar->High < lower && price < LL && position() >= 0) {
		if (position() > 0) {
			sellToClose(position());
		}
		if (Lots > 0 && Trader::Instance().canOpen() && !isLowerLimited()) {
			sell(Lots);
			mStopPrice = price + S;
		}
	}
}

void BollBreak::onTimer(const ptime& time) {

}

void BollBreak::onBarStart(const Chart* pChart) {
	if (Chart::PT_PERIOD_M15 != pChart->getPeriodType()) {
		return;
	}

	const Chart* pDayChart = getChart(Chart::PT_PERIOD_D1);
	if (pDayChart->getBarCount() < (size_t)N + 1) {
		return;
	}

	if (!mInitialized) {
		double price = pChart->getBar(0)->Close;
		double money = Trader::Instance().getMoney();
		S = getMargin() * 8 * 0.012 * price;
		Lots = (int)roundl(money * 0.125 / (price * getQuantity() * getMargin()));

		mInitialized = true;
	}

	double MA = pDayChart->Average(1, N, Bar::PT_CLOSE);
	double band = pDayChart->StandardDev(1, N, Bar::PT_CLOSE);

	upper = MA + K * band;
	lower = MA - K * band;
	HH = pDayChart->Highest(1, 2, Bar::PT_HIGH);
	LL = pDayChart->Lowest(1, 2, Bar::PT_LOW);

	m_pLastBar = pChart->getBar(1);
}