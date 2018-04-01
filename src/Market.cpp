#include <fstream>
#include "Market.h"
#include "TimeUtil.h"
#include "Contract.h"
#include "Trader.h"
#include "Logger.h"

using namespace std;
using namespace boost;
using namespace rapidjson;

Market::Market() {
}

Market::~Market() {
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		delete it->second;
	}
}

Contract* Market::addContract(const string& name, const string& timeKey, double tickPrice, bool isShangHai, int quantity, double margin) {
	auto it = mContracts.find(name);
	if (it == mContracts.end()) {
		Contract* pContract = new Contract(name, timeKey, tickPrice, isShangHai, quantity, margin);
		mContracts[name] = pContract;

		return pContract;
	}

	return it->second;
}

Contract* Market::getContract(const std::string& name) {
	auto it = mContracts.find(name);
	if (it != mContracts.end()) {
		return it->second;
	}
	
	return nullptr;
}

void Market::checkOrder() {
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		it->second->removeOrder();
	}
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		if (!Trader::Instance().canSendRequest()) {
			return;
		}
		it->second->insertOrder();
		if (!Trader::Instance().canSendRequest()) {
			return;
		}
		it->second->cancelOrder();
	}
}

void Market::onRobotTick() {
	vector<Contract*> contracts;
	vector<Contract*> positions;

	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		if (it->second->hasPosition()) {
			positions.push_back(it->second);
		}
		else {
			contracts.push_back(it->second);
		}
	}

	for (auto it = positions.begin(); it != positions.end(); ++it) {
		(*it)->onRobotTick();
	}
	positions.clear();

	size_t s = contracts.size();
	while (s > 0)
	{
		auto it = contracts.begin() + (rand() % s);
		(*it)->onRobotTick();
		contracts.erase(it);
		s = contracts.size();
	}
}

void Market::resetTradingDay() {
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		it->second->resetTradingDay();
	}
}

void Market::read() {
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		it->second->read();
	}
}

void Market::save() {
	Logger::Info() >> "-------- enter Market::save --------";
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		it->second->save();
	}

	ofstream fs("./config/Record/Order.csv", ofstream::app);

	if (!fs.good()) {
		fs.close();
		return;
	}

	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		Contract* pContract = it->second;
		list<Order*>& orders = pContract->getFinishOrders();
		for (auto i = orders.begin(); i != orders.end(); ++i) {
			Order* pOrder = *i;
			fs << pOrder->toString() << endl;
			delete pOrder;
		}
		orders.clear();
	}

	fs.close();
}

Value Market::saveDoc(Document& doc) {
	Value v(kObjectType);
	
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		v.AddMember(Value(it->first.c_str(), doc.GetAllocator()), it->second->saveDoc(doc), doc.GetAllocator());
	}

	return v;
}

void Market::readDoc(const Value& v) {
	for (auto it = mContracts.begin(); it != mContracts.end(); ++it) {
		Value::ConstMemberIterator iv = v.FindMember(it->first.c_str());
		if (iv != v.MemberEnd()) {
			it->second->readDoc(iv->value);
		}
	}
}
