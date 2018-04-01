#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>


class TimeWindow
{
public:
	TimeWindow(const boost::gregorian::date& d, const std::string& td1, const std::string& td2);

	TimeWindow(const std::string& tStr1, const std::string& tStr2);

	bool contain(const boost::posix_time::ptime& time) const;

	bool contain(const boost::posix_time::time_duration& td) const;

	inline void shift(const boost::posix_time::time_duration& td) {
		m_pTP->shift(td);
	}

private:
	boost::posix_time::time_period* m_pTP;
};

class TimeSet {
public:
	TimeSet();

	void addTimeWindow(const boost::gregorian::date& d, const std::string& td1, const std::string& td2);

	void addTimeWindow(const std::string& tStr1, const std::string& tStr2);

	bool contain(const boost::posix_time::ptime& time) const;

	bool contain(const boost::posix_time::time_duration& td) const;

	void shift(const boost::posix_time::time_duration& td);

private:
	std::vector<TimeWindow*> mTimeWnds;
};

class TimeUtil
{
public:
	static TimeUtil& Instance();

	void update();

	const boost::posix_time::ptime& getTime();

	void adjustTime(const std::string& time);

	static boost::posix_time::ptime getPeriodStart(const boost::posix_time::ptime& time, const boost::posix_time::time_duration& duration);

	//time:"hh:mm:ss"
	boost::posix_time::ptime getActionTime(const std::string& time);

	bool isWorkingTime();

	bool isTradingTime(const std::string& key, const boost::posix_time::ptime& time);

	bool isTradingTime(const std::string& key, const boost::posix_time::time_duration& td);

private:
	TimeUtil();

	void parse();

	static TimeUtil* m_pInstance;

	boost::gregorian::date mStartDate;
	boost::posix_time::ptime mTime;
	boost::posix_time::time_duration mTimeOffset;

	TimeSet mWorkTime;
	TimeSet mHoliday;
	std::map<std::string, TimeSet*> mTradingTime;
};

#endif
