#if !defined(DUAL_TRUST_H)
#define DUAL_TRUST_H

#include "Robot.h"

class DualTrust : public Robot
{
public:
	DualTrust(const rapidjson::Value& params, int id);
	virtual ~DualTrust();

	virtual void readDoc(const rapidjson::Value& v);
	virtual rapidjson::Value saveDoc(rapidjson::Document& doc);

	virtual void onTick(const boost::posix_time::ptime& time, double lastPrice,
		double askPrice, int askVolume, double bidPrice, int bidVolume);
	virtual void onTimer(const boost::posix_time::ptime& time);
	virtual void onBarStart(const Chart* pChart);

private:
	int N;
	double K;
	int Lots;
};

#endif
