#if !defined(ROBOT_H)
#define ROBOT_H

#include <boost/lexical_cast.hpp>
#include "rapidjson/document.h"
#include "Contract.h"
#include "TimeUtil.h"


class Robot
{
public:
	Robot(const std::string& name, int id) : mName(name), mID(id)
	{
		mPosition = 0;
	}

	void setContract(Contract* pContract) {
		m_pContract = pContract;

		mKey = mName;
		mKey.push_back('_');
		mKey.append(m_pContract->getName());
		mKey.push_back('_');
		mKey.append(boost::lexical_cast<std::string>(mID));
	}

	inline const std::string& getName() const {
		return mName;
	}

	inline const std::string& getContractName() const {
		return m_pContract->getName();
	}

	virtual ~Robot() {}

	inline const std::string& getKey() {
		return mKey;
	}

	virtual void readDoc(const rapidjson::Value& v) = 0;
	virtual rapidjson::Value saveDoc(rapidjson::Document& doc) = 0;

	virtual void onTick(const boost::posix_time::ptime& time, double price,
		double askPrice, int askVolume, double bidPrice, int bidVolume) {
	}

	virtual void onTimer(const boost::posix_time::ptime& time) {
	}

	virtual void onBarStart(const Chart* pChart) {
	}

	int position() const {
		return mPosition;
	}

	void clearPosition(int volume) {
		if (mPosition > 0) {
			mPosition -= volume;
		}
		else if (mPosition < 0) {
			mPosition += volume;
		}
	}

protected:
	bool buy(int volume) {
		if (!TimeUtil::Instance().isTradingTime(m_pContract->getTimeKey(), TimeUtil::Instance().getTime())) {
			return false;
		}
		if (mPosition < 0 || volume == 0) {
			return false;
		}
		mPosition += volume;
		m_pContract->buy(this, volume);
		return true;
	}

	bool sell(int volume) {
		if (!TimeUtil::Instance().isTradingTime(m_pContract->getTimeKey(), TimeUtil::Instance().getTime())) {
			return false;
		}
		if (mPosition > 0 || volume == 0) {
			return false;
		}
		mPosition -= volume;
		m_pContract->sell(this, volume);
		return true;
	}

	bool buyToClose(int volume) {
		if (!TimeUtil::Instance().isTradingTime(m_pContract->getTimeKey(), TimeUtil::Instance().getTime())) {
			return false;
		}
		if (mPosition + volume > 0) {
			return false;
		}

		mPosition += volume;
		m_pContract->buyToClose(this, volume);

		return true;
	}

	bool sellToClose(int volume) {
		if (!TimeUtil::Instance().isTradingTime(m_pContract->getTimeKey(), TimeUtil::Instance().getTime())) {
			return false;
		}
		if (mPosition - volume < 0) {
			return false;
		}

		mPosition -= volume;
		m_pContract->sellToClose(this, volume);

		return true;
	}

	void setPosition(int position) {
		mPosition = position;
	}

	const Chart* getChart(Chart::PeriodType pt) {
		return m_pContract->getChart(pt);
	}

	double getTickPrice() const {
		return m_pContract->getTickPrice();
	}

	bool isUpperLimited() const {
		return m_pContract->isUpperLimited();
	}

	bool isLowerLimited() const {
		return m_pContract->isLowerLimited();
	}

	int getQuantity() const {
		return m_pContract->getQuantity();
	}

	double getMargin() const {
		return m_pContract->getMargin();
	}

private:
	std::string mKey;
	std::string mName;
	int mID;
	Contract* m_pContract;
	int mPosition;
};

#endif