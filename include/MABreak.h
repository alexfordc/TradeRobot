#if !defined(MA_BREAK_H)
#define MA_BREAK_H

#include "Robot.h"

class MABreak : public Robot
{
public:
	MABreak(const rapidjson::Value& params, int id);
	virtual ~MABreak();

	virtual void readDoc(const rapidjson::Value& v);
	virtual rapidjson::Value saveDoc(rapidjson::Document& doc);

	virtual void onTick(const boost::posix_time::ptime& time, double lastPrice,
		double askPrice, int askVolume, double bidPrice, int bidVolume);
	virtual void onTimer(const boost::posix_time::ptime& time);
	virtual void onBarStart(const Chart* pChart);

private:
	int Lots;
	double K;
	int N;
	double mStopPrice;
	bool mInitialized;
	double MA;
	double HH;
	double LL;
	int MAState;
};

#endif
