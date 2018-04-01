#if !defined(CHART_H)
#define CHART_H

#include <boost\date_time\posix_time\posix_time.hpp>
#include <vector>
#include "rapidjson/document.h"
#include "TickData.h"
#include "Bar.h"


class Chart {
public:
	enum PeriodType
	{
		PT_PERIOD_M1 = 1,
		PT_PERIOD_M3,
		PT_PERIOD_M5,
		PT_PERIOD_M10,
		PT_PERIOD_M15,
		PT_PERIOD_M30,
		PT_PERIOD_H1,
		PT_PERIOD_H2,
		PT_PERIOD_H4,
		PT_PERIOD_D1
	};

	Chart(const std::string& contract, PeriodType pt);

	~Chart();

	bool mergeTick(const TickData* pTick);

	void save();

	void read();

	rapidjson::Value saveDoc(rapidjson::Document& doc);

	void readDoc(const rapidjson::Value& v);

	inline const Bar* getBar(std::size_t i) const {
		std::size_t nLen = mBars.size();
		if (i < nLen) {
			return mBars[nLen - 1 - i];
		}
		else {
			return nullptr;
		}
	}

	inline std::size_t getBarCount() const {
		return mBars.size();
	}

	double Average(int nStart, int count, Bar::PriceType pt) const;

	double StandardDev(int nStart, int count, Bar::PriceType pt) const;

	double Highest(int nStart, int count, Bar::PriceType pt) const;

	double Lowest(int nStart, int count, Bar::PriceType pt) const;

	inline PeriodType getPeriodType() const {
		return mPT;
	}

	inline void resetDayVolume() {
		mLastDayVolume = 0;
		mLastBarDayVolume = 0;
		mTodayStartBarIdx = mBars.size();
	}

	inline std::size_t getTodayBarCount() const {
		return mBars.size() - mTodayStartBarIdx;
	}

private:
	std::string getFilename();

	std::string mContract;
	int mLastDayVolume;
	int mLastBarDayVolume;
	std::size_t mTodayStartBarIdx;
	PeriodType mPT;
	boost::posix_time::time_duration mPeriod;

	std::size_t mSaveCount;
	std::vector<Bar*> mBars;
};

#endif
