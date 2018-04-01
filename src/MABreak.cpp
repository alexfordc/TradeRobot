#include "MABreak.h"
#include <boost/filesystem.hpp>
#include "TimeUtil.h"
#include "Logger.h"
#include "Trader.h"

using namespace std;
using namespace rapidjson;
using namespace boost;
using namespace boost::posix_time;


MABreak::MABreak(const Value& params, int id) : Robot("MABreak", id)
{
	mStopPrice = 0.0;
	Lots = 0;
	N = 16;
	K = 100.0;
	mInitialized = false;
	MAState = 0;
}

MABreak::~MABreak()
{
}

void MABreak::readDoc(const Value& v) {
	setPosition(v["Position"].GetInt());
	mStopPrice = v["StopPrice"].GetDouble();
}

Value MABreak::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("Position", position(), doc.GetAllocator());
	v.AddMember("StopPrice", mStopPrice, doc.GetAllocator());

	return v;
}

// Order: onBarStart -> onTick -> onTimer
void MABreak::onTick(const ptime& time, double price,
	double askPrice, int askVolume, double bidPrice, int bidVolume) {
	if (!mInitialized || Lots == 0) {
		return;
	}

	if (position() > 0 && (price < mStopPrice || MAState < 0)) {
		sellToClose(position());
	}

	if (position() < 0 && (price > mStopPrice || MAState > 0)) {
		buyToClose(-position());
	}

	if (MAState > 0 && price > HH && position() <= 0) {
		if (position() < 0) {
			buyToClose(-position());
		}
		if (Lots > 0 && Trader::Instance().canOpen() && !isUpperLimited()) {
			buy(Lots);
			mStopPrice = price - K;
		}
	}

	if (MAState < 0 && price < LL && position() >= 0) {
		if (position() > 0) {
			sellToClose(position());
		}
		if (Lots > 0 && Trader::Instance().canOpen() && !isLowerLimited()) {
			sell(Lots);
			mStopPrice = price + K;
		}
	}
}

void MABreak::onTimer(const ptime& time) {

}

void MABreak::onBarStart(const Chart* pChart) {
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
		double LotsRatio = Trader::Instance().getLotsRatio();
		Lots = (int)roundl(money * 0.125 / (price * getQuantity() * getMargin()) * LotsRatio);
		K = getMargin() * 8 * 0.012 * price;
		N = Trader::Instance().getParam1();

		mInitialized = true;
	}

	MA = pDayChart->Average(1, N, Bar::PT_CLOSE);
	HH = pDayChart->Highest(1, 2, Bar::PT_HIGH);
	LL = pDayChart->Lowest(1, 2, Bar::PT_LOW);

	size_t n = pChart->getTodayBarCount();
	if (n > 1) {
		HH = max(HH, pChart->Highest(1, n - 1, Bar::PT_HIGH));
		LL = min(LL, pChart->Lowest(1, n - 1, Bar::PT_LOW));
	}

	const Bar* pLastBar = pChart->getBar(1);
	if (pLastBar->Low > MA) {
		MAState = 1;
	}
	else if (pLastBar->High < MA) {
		MAState = -1;
	}
	else {
		MAState = 0;
	}
}