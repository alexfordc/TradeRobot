#if !defined(MARKET_H)
#define MARKET_H

#include <map>
#include "rapidjson/document.h"
#include "ctp/ThostFtdcUserApiStruct.h"

class Contract;

class Market
{
public:
	Market();

	~Market();

	Contract* addContract(const std::string& name, const std::string& timeKey, double tickPrice, bool isShangHai, int quantity, double margin);

	Contract* getContract(const std::string& name);

	inline size_t getContractsCount() {
		return mContracts.size();
	}

	inline const std::map<std::string, Contract*>& getContracts() const {
		return mContracts;
	}

	void checkOrder();

	void onRobotTick();

	void resetTradingDay();

	void save();

	void read();

	rapidjson::Value saveDoc(rapidjson::Document& doc);

	void readDoc(const rapidjson::Value& v);

private:
	std::map<std::string, Contract*> mContracts;
};

#endif
