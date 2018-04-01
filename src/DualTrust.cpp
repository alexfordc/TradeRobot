#include "DualTrust.h"
#include <boost/filesystem.hpp>
#include "TimeUtil.h"
#include "Logger.h"

using namespace std;
using namespace rapidjson;
using namespace boost;
using namespace boost::posix_time;


DualTrust::DualTrust(const Value& params, int id) : Robot("DualTrust", id)
{
	K = params["K"].GetDouble();
	N = params["N"].GetInt();
	Lots = params["Lots"].GetInt();
}

DualTrust::~DualTrust()
{
}

void DualTrust::readDoc(const Value& v) {
	setPosition(v["Position"].GetInt());
}

Value DualTrust::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("Position", position(), doc.GetAllocator());

	return v;
}

// Order: onBarStart -> onTick -> onTimer
void DualTrust::onTick(const ptime& time, double lastPrice,
	double askPrice, int askVolume, double bidPrice, int bidVolume) {
}

void DualTrust::onTimer(const ptime& time) {
}

void DualTrust::onBarStart(const Chart* pChart) {
	if (Chart::PT_PERIOD_M5 != pChart->getPeriodType()) {
		return;
	}

	const Chart* pDayChart = getChart(Chart::PT_PERIOD_D1);
	if (pDayChart->getBarCount() < (size_t)N + 2) {
		return;
	}

	int start = pChart->getTodayBarCount() == 1 ? 2 : 1;

	double HH = pDayChart->Highest(start, N, Bar::PT_HIGH);
	double HC = pDayChart->Highest(start, N, Bar::PT_CLOSE);
	double LL = pDayChart->Lowest(start, N, Bar::PT_LOW);
	double LC = pDayChart->Lowest(start, N, Bar::PT_CLOSE);

	double todayOpen = pDayChart->getBar(start-1)->Open;
	double range = K * max(HH - LC, HC - LL);
	double UpperLine = todayOpen + range;
	double LowerLine = todayOpen - range;

	const Bar* pBar = pChart->getBar(1);

	if (pBar->Low >= UpperLine && position() <= 0) {
		if (position() < 0) {
			buyToClose(-position());
		}
		if (Lots > 0) {
			buy(Lots);
		}
	}

	if (pBar->High <= LowerLine && position() >= 0) {
		if (position() > 0) {
			sellToClose(position());
		}
		if (Lots > 0) {
			sell(Lots);
		}
	}
}