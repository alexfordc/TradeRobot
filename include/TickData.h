#if !defined(TICKDATA_H)
#define TICKDATA_H

#include "TimeUtil.h"
#include "ctp/ThostFtdcUserApiStruct.h"

struct TickData {
	TickData() {
		Time = boost::posix_time::ptime(boost::posix_time::min_date_time);
	}

	TickData(CThostFtdcDepthMarketDataField* pTick) {
		init(pTick);
	}

	void init(CThostFtdcDepthMarketDataField* pTick) {
		InstrumentID = pTick->InstrumentID;
		Time = TimeUtil::Instance().getActionTime(pTick->UpdateTime);
		AskPrice = pTick->AskPrice1;
		AskVolume = pTick->AskVolume1;
		BidPrice = pTick->BidPrice1;
		BidVolume = pTick->BidVolume1;
		Price = pTick->LastPrice;
		OpenInterest = pTick->OpenInterest;
		DayVolume = pTick->Volume;
		UpperLimitPrice = pTick->UpperLimitPrice;
		LowerLimitPrice = pTick->LowerLimitPrice;
		OpenPrice = pTick->OpenPrice;

		if (AskPrice < LowerLimitPrice || AskPrice > UpperLimitPrice) {
			AskPrice = Price;
		}

		if (BidPrice < LowerLimitPrice || BidPrice > UpperLimitPrice) {
			BidPrice = Price;
		}
	}

	void copy(const TickData& tick) {
		InstrumentID = tick.InstrumentID;
		Time = tick.Time;
		AskPrice = tick.AskPrice;
		AskVolume = tick.AskVolume;
		BidPrice = tick.BidPrice;
		BidVolume = tick.BidVolume;
		Price = tick.Price;
		OpenInterest = tick.OpenInterest;
		DayVolume = tick.DayVolume;
		UpperLimitPrice = tick.UpperLimitPrice;
		LowerLimitPrice = tick.LowerLimitPrice;
	}

	std::string InstrumentID;
	boost::posix_time::ptime Time;
	double AskPrice;
	int AskVolume;
	double BidPrice;
	int BidVolume;
	double Price;
	double OpenPrice;
	double OpenInterest;
	int DayVolume;
	double UpperLimitPrice;
	double LowerLimitPrice;
};

#endif
