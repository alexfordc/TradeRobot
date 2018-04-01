#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "Chart.h"
#include "TimeUtil.h"
#include "Logger.h"
#include "Global.h"
#include "Util.h"

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace rapidjson;

Chart::Chart(const string& contract, PeriodType pt) 
	:mContract(contract), mPT(pt)
{
	switch (mPT)
	{
	case PT_PERIOD_M1:
		mPeriod = minutes(1);
		break;
	case PT_PERIOD_M3:
		mPeriod = minutes(3);
		break;
	case PT_PERIOD_M5:
		mPeriod = minutes(5);
		break;
	case PT_PERIOD_M10:
		mPeriod = minutes(10);
		break;
	case PT_PERIOD_M15:
		mPeriod = minutes(15);
		break;
	case PT_PERIOD_M30:
		mPeriod = minutes(30);
		break;
	case PT_PERIOD_H1:
		mPeriod = hours(1);
		break;
	case PT_PERIOD_H2:
		mPeriod = hours(2);
		break;
	case PT_PERIOD_H4:
		mPeriod = hours(4);
		break;
	case PT_PERIOD_D1:
		mPeriod = hours(24);
		break;
	default:
		break;
	}
	mSaveCount = 0;
	mLastDayVolume = 0;
	mLastBarDayVolume = 0;
	mTodayStartBarIdx = 0;
}

Chart::~Chart() {
	for (auto it = mBars.begin(); it != mBars.end(); ++it) {
		delete *it;
	}
	mBars.clear();
}

bool Chart::mergeTick(const TickData* pTick) {
	if (mLastDayVolume >= pTick->DayVolume) {
		return false;
	}

	bool isNewBar = false;

	if (mBars.empty()) {
		isNewBar = true;
	}
	else {
		Bar* pBar = mBars.back();
		if (mPeriod.hours() == 24) {
			if (Global::getTradingDay() != pBar->Time.date()) {
				isNewBar = true;
			}
		}
		else {
			if (pTick->Time - pBar->Time >= mPeriod) {
				isNewBar = true;
			}
		}
	}

	Bar* pBar = nullptr;

	if (isNewBar) {
		mLastBarDayVolume = mLastDayVolume;

		pBar = new Bar();

		if (mPeriod.hours() == 24) {
			pBar->Time = ptime(Global::getTradingDay());
		}
		else {
			pBar->Time = TimeUtil::getPeriodStart(pTick->Time, mPeriod);
		}
		
		if (mLastDayVolume == 0) {
			pBar->Open = pTick->OpenPrice;
		}
		else {
			pBar->Open = pTick->Price;
		}

		pBar->Close = pTick->Price;
		pBar->High = pTick->Price;
		pBar->Low = pTick->Price;
		pBar->Volume = pTick->DayVolume - mLastBarDayVolume;
		pBar->OpenInterest = pTick->OpenInterest;
		pBar->LowerLimitPrice = pTick->LowerLimitPrice;
		pBar->UpperLimitPrice = pTick->UpperLimitPrice;

		mBars.push_back(pBar);
	}
	else {
		pBar = mBars.back();

		pBar->Close = pTick->Price;
		pBar->Volume = pTick->DayVolume - mLastBarDayVolume;
		pBar->OpenInterest = pTick->OpenInterest;

		if (pBar->High < pTick->Price) {
			pBar->High = pTick->Price;
		}
		if (pBar->Low > pTick->Price) {
			pBar->Low = pTick->Price;
		}
	}

	mLastDayVolume = pTick->DayVolume;

	return isNewBar;
}

string Chart::getFilename() {
	string path = "./config/Market/";
	path.append(mContract);

	if (mPeriod.hours() == 24) {
		path.append("_D1.csv");
	}
	else if (mPeriod.hours() > 1) {
		path.append("_H");
		path.append(lexical_cast<string>(mPeriod.hours()));
		path.append(".csv");
	}
	else {
		path.append("_M");
		path.append(lexical_cast<string>(mPeriod.minutes()));
		path.append(".csv");
	}

	return path;
}

void Chart::save() {
	if (mBars.empty()) {
		return;
	}

	string path = getFilename();
	ofstream fs;
	
	if (mPeriod.hours() == 24) {
		fs.open(path);
	}
	else {
		fs.open(path, ofstream::app);
	}

	if (!fs.good()) {
		fs.close();
		Logger::Error() << path >> " Save Failed!";
		return;
	}

	size_t nLen = mBars.size();

	if (mPeriod.hours() == 24) {
		mSaveCount = 0;
	}

	for (size_t i = mSaveCount; i != nLen; ++i) {
		Bar* pBar = mBars[i];

		fs << to_simple_string(pBar->Time) << ",";
		fs << pBar->Open << "," << pBar->High << ",";
		fs << pBar->Low << "," << pBar->Close << ",";
		fs << pBar->Volume << "," << pBar->OpenInterest << ",";
		fs << pBar->LowerLimitPrice << "," << pBar->UpperLimitPrice << endl;
	}

	fs.close();

	mSaveCount = nLen;
}

void Chart::read() {
	if (!mBars.empty()) {
		return ;
	}

	string text;
	if (!Util::readFile(getFilename(), text)) {
		return ;
	}

	vector<string> lines;
	boost::split(lines, text, boost::is_any_of("\n\r"), token_compress_on);

	for (auto it = lines.begin(); it != lines.end(); ++it)
	{
		string& line = *it;
		vector<string> words;

		if (line.length() > 0) {
			boost::split(words, line, boost::is_any_of(","), token_compress_on);
		}

		if (words.empty()) {
			continue;
		}

		Bar* pBar = new Bar();

		pBar->Time = time_from_string(words[0]);
		pBar->Open = lexical_cast<double>(words[1]);
		pBar->High = lexical_cast<double>(words[2]);
		pBar->Low = lexical_cast<double>(words[3]);
		pBar->Close = lexical_cast<double>(words[4]);
		pBar->Volume = lexical_cast<int>(words[5]);
		pBar->OpenInterest = lexical_cast<double>(words[6]);
		pBar->LowerLimitPrice = lexical_cast<double>(words[7]);
		pBar->UpperLimitPrice = lexical_cast<double>(words[8]);

		mBars.push_back(pBar);
	}

	mSaveCount = mBars.size();
}

Value Chart::saveDoc(Document& doc) {
	Value v(kObjectType);

	v.AddMember("LastDayVolume", mLastDayVolume, doc.GetAllocator());
	v.AddMember("LastBarDayVolume", mLastBarDayVolume, doc.GetAllocator());
	v.AddMember("TodayStartBarIdx", mTodayStartBarIdx, doc.GetAllocator());

	return v;
}

void Chart::readDoc(const Value& v) {
	mLastDayVolume = v["LastDayVolume"].GetInt();
	mLastBarDayVolume = v["LastBarDayVolume"].GetInt();
	Value::ConstMemberIterator it = v.FindMember("TodayStartBarIdx");
	if (it != v.MemberEnd()) {
		mTodayStartBarIdx = it->value.GetInt();
	}
}

double Chart::Average(int nStart, int count, Bar::PriceType pt) const {
	if (nStart < 0 || count < 1) {
		return 0.0;
	}
	double price = 0.0;
	switch (pt)
	{
	case Bar::PT_CLOSE:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			price += pBar->Close;
		}
		break;
	case Bar::PT_OPEN:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			price += pBar->Open;
		}
		break;
	case Bar::PT_HIGH:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			price += pBar->High;
		}
		break;
	case Bar::PT_LOW:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			price += pBar->Low;
		}
		break;
	default:
		break;
	}

	price /= count;

	return price;
}

double Chart::StandardDev(int nStart, int count, Bar::PriceType pt) const {
	if (nStart < 0 || count < 2) {
		return 0.0;
	}

	double average = Average(nStart, count, pt);

	double dev = 0.0;
	switch (pt)
	{
	case Bar::PT_CLOSE:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			dev += (pBar->Close - average) * (pBar->Close - average);
		}
		break;
	case Bar::PT_OPEN:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			dev += (pBar->Open - average) * (pBar->Open - average);
		}
		break;
	case Bar::PT_HIGH:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			dev += (pBar->High - average) * (pBar->High - average);
		}
		break;
	case Bar::PT_LOW:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			dev += (pBar->Low - average) * (pBar->Low - average);
		}
		break;
	default:
		break;
	}

	dev /= count;

	return sqrt(dev);
}

double Chart::Highest(int nStart, int count, Bar::PriceType pt) const {
	if (nStart < 0 || count < 1) {
		return 0.0;
	}

	double highest = 0.0;

	switch (pt)
	{
	case Bar::PT_CLOSE:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			highest = max(highest, pBar->Close);
		}
		break;
	case Bar::PT_OPEN:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			highest = max(highest, pBar->Open);
		}
		break;
	case Bar::PT_HIGH:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			highest = max(highest, pBar->High);
		}
		break;
	case Bar::PT_LOW:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			highest = max(highest, pBar->Low);
		}
		break;
	default:
		break;
	}

	return highest;
}

double Chart::Lowest(int nStart, int count, Bar::PriceType pt) const {
	if (nStart < 0 || count < 1) {
		return 0.0;
	}

	double lowest = 100000000.0;

	switch (pt)
	{
	case Bar::PT_CLOSE:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			lowest = min(lowest, pBar->Close);
		}
		break;
	case Bar::PT_OPEN:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			lowest = min(lowest, pBar->Open);
		}
		break;
	case Bar::PT_HIGH:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			lowest = min(lowest, pBar->High);
		}
		break;
	case Bar::PT_LOW:
		for (int i = 0; i < count; ++i) {
			const Bar* pBar = getBar(nStart + i);
			lowest = min(lowest, pBar->Low);
		}
		break;
	default:
		break;
	}

	return lowest;
}