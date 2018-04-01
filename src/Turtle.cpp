#include <boost/filesystem.hpp>
#include "Turtle.h"
#include "TimeUtil.h"
#include "Logger.h"
#include "Trader.h"

using namespace std;
using namespace rapidjson;
using namespace boost;
using namespace boost::posix_time;

Turtle::Turtle(const Value& params, int id) : Robot("Turtle", id)
{
	mStopPrice = 0.0;
	Lots = 0;
	K = 100.0;
}

Turtle::~Turtle()
{
}

void Turtle::readDoc(const Value& v) {
	setPosition(v["Position"].GetInt());
	mStopPrice = v["StopPrice"].GetDouble();
}

Value Turtle::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("Position", position(), doc.GetAllocator());
	v.AddMember("StopPrice", mStopPrice, doc.GetAllocator());

	return v;
}

// Order: onBarStart -> onTick -> onTimer
void Turtle::onTick(const ptime& time, double price,
	double askPrice, int askVolume, double bidPrice, int bidVolume) {

	if (Lots == 0) {
		double money = Trader::Instance().getMoney();
		double LotsRatio = Trader::Instance().getLotsRatio();
		Lots = (int)roundl(money * 0.125 / (price * getQuantity() * getMargin()) * LotsRatio);
		K = money * 0.02 / (Lots * getQuantity());
		N1 = Trader::Instance().getParam1();
		N2 = Trader::Instance().getParam2();
	}

	if (Lots == 0) {
		return;
	}

	const Chart* pChart = getChart(Chart::PT_PERIOD_D1);
	if (pChart->getBarCount() <= (unsigned int)N1) {
		return;
	}

	const Bar* pBar = pChart->getBar(0);

	double HH1 = pChart->Highest(1, N1, Bar::PT_HIGH);
	double LL1 = pChart->Lowest(1, N1, Bar::PT_LOW);
	double HH2 = pChart->Highest(1, N2, Bar::PT_HIGH);
	double LL2 = pChart->Lowest(1, N2, Bar::PT_LOW);
	double close = pBar->getPrice(Bar::PT_CLOSE);
	double high = pBar->getPrice(Bar::PT_HIGH);
	double low = pBar->getPrice(Bar::PT_LOW);
	double oneTick = getTickPrice();
	
	if (position() == 0) {
		if (close > HH1 && close+oneTick > high) {
			if (Lots > 0 && Trader::Instance().canOpen() && !isUpperLimited()) {
				buy(Lots);
				mStopPrice = close - K;
			}
		}
		else if (close < LL1 && close-oneTick < low) {
			if (Lots > 0 && Trader::Instance().canOpen() && !isLowerLimited()) {
				sell(Lots);
				mStopPrice = close + K;
			}
		}
	}
	else {
		if (position() > 0 && (close < LL2 || close < mStopPrice)) {
			sellToClose(position());
		}

		if (position() < 0 && (close > HH2 || close > mStopPrice)) {
			buyToClose(-position());
		}
	}
}

void Turtle::onTimer(const ptime& time) {
}

void Turtle::onBarStart(const Chart* pChart) {
}