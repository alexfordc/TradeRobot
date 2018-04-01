#if !defined(BAR_H)
#define BAR_H

#include <boost\date_time\posix_time\posix_time.hpp>

struct Bar
{
	enum PriceType
	{
		PT_CLOSE = 1,
		PT_OPEN,
		PT_HIGH,
		PT_LOW
	};

	double Open;
	double Close;
	double High;
	double Low;
	double OpenInterest;
	int Volume;
	double UpperLimitPrice;
	double LowerLimitPrice;
	boost::posix_time::ptime Time;

	double getPrice(PriceType pt) const {
		switch (pt)
		{
		case PT_CLOSE:
			return Close;
			break;
		case PT_OPEN:
			return Open;
			break;
		case PT_HIGH:
			return High;
			break;
		case PT_LOW:
			return Low;
			break;
		default:
			return 0.0;
			break;
		}
	}
};

#endif