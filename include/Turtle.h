#if !defined(TURTLE_H)
#define TURTLE_H

#include "Robot.h"

class Turtle : public Robot
{
public:
	Turtle(const rapidjson::Value& params, int id);
	virtual ~Turtle();

	virtual void readDoc(const rapidjson::Value& v);
	virtual rapidjson::Value saveDoc(rapidjson::Document& doc);

	virtual void onTick(const boost::posix_time::ptime& time, double lastPrice,
		double askPrice, int askVolume, double bidPrice, int bidVolume);
	virtual void onTimer(const boost::posix_time::ptime& time);
	virtual void onBarStart(const Chart* pChart);

private:
	int Lots;
	double K;
	int N1;
	int N2;
	double mStopPrice;
};

#endif
