#include "rapidjson/document.h"
#include "TimeUtil.h"
#include "Util.h"

using namespace std;
using namespace boost;
using namespace rapidjson;
using namespace boost::posix_time;
using namespace boost::gregorian;

TimeWindow::TimeWindow(const date& d, const string& td1, const string& td2) {
	ptime t1(d, duration_from_string(td1));
	ptime t2(d, duration_from_string(td2));

	m_pTP = new time_period(t1, t2);
}

TimeWindow::TimeWindow(const string& tStr1, const string& tStr2) {
	ptime t1(time_from_string(tStr1));
	ptime t2(time_from_string(tStr2));

	m_pTP = new time_period(t1, t2);
}

bool TimeWindow::contain(const ptime& time) const {
	return m_pTP->contains(time);
}

bool TimeWindow::contain(const time_duration& td) const {
	time_duration td1 = m_pTP->begin().time_of_day();
	time_duration dt = m_pTP->length();
	time_duration td2 = td1 + dt;

	return td1 <= td && td <= td2;
}

TimeSet::TimeSet() {
}

bool TimeSet::contain(const ptime& time) const {
	for (auto it = mTimeWnds.begin(); it != mTimeWnds.end(); ++it) {
		if ((*it)->contain(time)) {
			return true;
		}
	}

	return false;
}

bool TimeSet::contain(const time_duration& td) const {
	for (auto it = mTimeWnds.begin(); it != mTimeWnds.end(); ++it) {
		if ((*it)->contain(td)) {
			return true;
		}
	}

	return false;
}

void TimeSet::addTimeWindow(const date& d, const string& td1, const string& td2) {
	TimeWindow* pts = new TimeWindow(d, td1, td2);
	mTimeWnds.push_back(pts);
}

void TimeSet::addTimeWindow(const string& tStr1, const string& tStr2) {
	TimeWindow* pts = new TimeWindow(tStr1, tStr2);
	mTimeWnds.push_back(pts);
}

void TimeSet::shift(const time_duration& td) {
	for (auto it = mTimeWnds.begin(); it != mTimeWnds.end(); ++it) {
		(*it)->shift(td);
	}
}


TimeUtil* TimeUtil::m_pInstance = nullptr;

TimeUtil& TimeUtil::Instance() {
	if (m_pInstance != nullptr) {
		return *m_pInstance;
	}

	m_pInstance = new TimeUtil();

	return *m_pInstance;
}

TimeUtil::TimeUtil() {
	mTime = second_clock::local_time();

	mStartDate = day_clock::local_day();
	while (mStartDate.day_of_week() != Monday) {
		mStartDate -= date_duration(1);
	}

	parse();
}

void TimeUtil::parse() {
	string text;
	Util::readFile("./config/ConfigSchedule.json", text);

	Document doc;

	doc.Parse(text.c_str());

	Value::MemberIterator it = doc.FindMember("WorkTime");
	if (it != doc.MemberEnd()) {
		for (Value::ValueIterator i = it->value.GetArray().Begin(); i != it->value.GetArray().End(); ++i) {
			mWorkTime.addTimeWindow(mStartDate, (*i)["Start"].GetString(), (*i)["End"].GetString());
		}
	}

	it = doc.FindMember("Holiday");
	if (it != doc.MemberEnd()) {
		for (Value::ValueIterator i = it->value.GetArray().Begin(); i != it->value.GetArray().End(); ++i) {
			string start = (*i)["Start"].GetString();
			string end = (*i)["End"].GetString();
			mHoliday.addTimeWindow(start, end);
		}
	}

	it = doc.FindMember("TradeTime");
	if (it != doc.MemberEnd()) {
		for (Value::ConstMemberIterator itm = it->value.MemberBegin(); itm != it->value.MemberEnd(); ++itm) {
			TimeSet* pTS = new TimeSet();
			mTradingTime[itm->name.GetString()] = pTS;
			for (Value::ConstValueIterator i = itm->value.GetArray().Begin(); i != itm->value.GetArray().End(); ++i) {
				pTS->addTimeWindow(mTime.date(), (*i)["Start"].GetString(), (*i)["End"].GetString());
			}
		}
	}
}

void TimeUtil::update() {
	date lastDate = mTime.date();
	mTime = second_clock::local_time() + mTimeOffset;
	date_duration dd1 = mTime.date() - lastDate;
	date_duration dd2 = mTime.date() - mStartDate;
	if (dd1.days() > 0) {
		time_duration td(hours(24 * dd1.days()));
		for (auto it = mTradingTime.begin(); it != mTradingTime.end(); ++it) {
			it->second->shift(td);
		}
	}
	if (dd2.days() >= 7) {
		mStartDate += days(7);
		time_duration td(hours(24 * 7));
		mWorkTime.shift(td);
	}
}

const ptime& TimeUtil::getTime() {
	return mTime;
}

void TimeUtil::adjustTime(const string& time) {
	mTimeOffset = getActionTime(time) - second_clock::local_time();
}

ptime TimeUtil::getPeriodStart(const ptime& time, const time_duration& duration) {
	if (duration <= hours(1)) {
		ptime t(time.date(), hours(time.time_of_day().hours()));

		while (t + duration <= time)
		{
			t += duration;
		}

		return t;
	}
	else if (duration < hours(24)) {
		ptime t(time.date());

		while (t + duration <= time)
		{
			t += duration;
		}

		return t;
	}
	else {
		ptime t(time.date());

		return t;
	}
}

ptime TimeUtil::getActionTime(const std::string& time) {
	time_duration td(duration_from_string(time));
	int h1 = mTime.time_of_day().hours();
	int h2 = td.hours();

	if (h1 - h2 > 12) {
		ptime t(mTime.date() + days(1));
		return t + td;
	}
	else if (h2 - h1 > 12) {
		ptime t(mTime.date() - days(1));
		return t + td;
	}
	else {
		ptime t(mTime.date());
		return t + td;
	}
}

bool TimeUtil::isWorkingTime() {
	return mWorkTime.contain(mTime) && (!mHoliday.contain(mTime));
}

bool TimeUtil::isTradingTime(const string& key, const ptime& time) {
	return mTradingTime[key]->contain(time);
}

bool TimeUtil::isTradingTime(const std::string& key, const time_duration& td) {
	return mTradingTime[key]->contain(mTime+td);
}
