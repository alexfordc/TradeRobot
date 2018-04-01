#if !defined(BOLL_BREAK_H)
#define BOLL_BREAK_H

#include "Robot.h"

class BollBreak : public Robot
{
public:
	BollBreak(const rapidjson::Value& params, int id);
	virtual ~BollBreak();

	virtual void readDoc(const rapidjson::Value& v);
	virtual rapidjson::Value saveDoc(rapidjson::Document& doc);

	virtual void onTick(const boost::posix_time::ptime& time, double lastPrice,
		double askPrice, int askVolume, double bidPrice, int bidVolume);
	virtual void onTimer(const boost::posix_time::ptime& time);
	virtual void onBarStart(const Chart* pChart);

private:
	int N;
	double K;
	double S;
	int Lots;
	double mStopPrice;
	bool mInitialized;
	double HH;
	double LL;
	double upper;
	double lower;
	const Bar* m_pLastBar;
};

#endif
