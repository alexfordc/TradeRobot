#include <boost/filesystem.hpp>
#include "SingleMA.h"
#include "TimeUtil.h"
#include "Logger.h"

using namespace std;
using namespace rapidjson;
using namespace boost;
using namespace boost::posix_time;


SingleMA::SingleMA(const Value& params, int id) : Robot("SingleMA", id)
{
	N = params["N"].GetInt();
	Lots = params["Lots"].GetInt();
}

SingleMA::~SingleMA()
{
}

void SingleMA::readDoc(const Value& v) {
	setPosition(v["Position"].GetInt());
}

Value SingleMA::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("Position", position(), doc.GetAllocator());

	return v;
}

// Order: onBarStart -> onTick -> onTimer
void SingleMA::onTick(const ptime& time, double lastPrice,
	double askPrice, int askVolume, double bidPrice, int bidVolume) {
}

void SingleMA::onTimer(const ptime& time) {

}

void SingleMA::onBarStart(const Chart* pChart) {
	if (Chart::PT_PERIOD_D1 != pChart->getPeriodType() ||
		pChart->getBarCount() < (size_t)N + 1) {
		return;
	}

	double average = pChart->Average(1, N, Bar::PT_CLOSE);
	
	const Bar* pBar = pChart->getBar(1);

	if (pBar->Low > average) {
		if (position() < 0) {
			buyToClose(-position());
		}
		buy(Lots);
	}

	if (pBar->High < average) {
		if (position() > 0) {
			sellToClose(position());
		}
		sell(Lots);
	}
}