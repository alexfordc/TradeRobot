#if !defined(ORDER_MSG_H)
#define ORDER_MSG_H

#include <string>
#include <boost/lexical_cast.hpp>

struct OrderMsg
{
	enum OrderMsgType {
		OMT_INSERT_REJECTED,
		OMT_CANCEL_REJECTED,
		OMT_CANCELED,
		OMT_TRADING,
		OMT_NOT_TRADING,
		OMT_ALL_TRADED,
		OMT_TRADE
	};

	OrderMsg(const std::string& instrumentId, int orderRef, OrderMsgType type,  int volume, double price) {
		InstrumentID = instrumentId;
		OrderRef = orderRef;
		Type = type;
		Volume = volume;
		Price = price;
	}

	std::string toString() const {
		std::string str = InstrumentID;
		str.push_back(',');
		str.append(boost::lexical_cast<std::string>(OrderRef)).push_back(',');
		str.append(boost::lexical_cast<std::string>(Type)).push_back(',');
		str.append(boost::lexical_cast<std::string>(Volume)).push_back(',');
		str.append(boost::lexical_cast<std::string>(Price));
		return str;
	}

	std::string InstrumentID;
	int OrderRef;
	OrderMsgType Type;
	int Volume;
	double Price;
};

#endif
