#if !defined(SINGLE_MA_H)
#define SINGLE_MA_H

#include "Robot.h"

class SingleMA : public Robot
{
public:
	SingleMA(const rapidjson::Value& params, int id);
	virtual ~SingleMA();

	virtual void readDoc(const rapidjson::Value& v);
	virtual rapidjson::Value saveDoc(rapidjson::Document& doc);

	virtual void onTick(const boost::posix_time::ptime& time, double lastPrice,
		double askPrice, int askVolume, double bidPrice, int bidVolume);
	virtual void onTimer(const boost::posix_time::ptime& time);
	virtual void onBarStart(const Chart* pChart);

private:
	int N;
	int Lots;
};

#endif
