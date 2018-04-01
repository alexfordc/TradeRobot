#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "Statistics.h"
#include "Util.h"
#include "Order.h"
#include "Logger.h"


using namespace std;
using namespace boost;

Statistics* Statistics::m_pInstance = nullptr;

Statistics& Statistics::Instance() {
	if (m_pInstance != nullptr) {
		return *m_pInstance;
	}

	m_pInstance = new Statistics();

	return *m_pInstance;
}

void Statistics::run() {
	string text;
	if (!Util::readFile("./config/Record/Order.csv", text)) {
		return;
	}

	map<string, ClipData*> result;

	vector<string> lines;
	boost::split(lines, text, boost::is_any_of("\n\r"), token_compress_on);

	for (auto it = lines.begin(); it != lines.end(); ++it)
	{
		string& line = *it;
		vector<string> words;

		if (line.length() > 0) {
			boost::split(words, line, boost::is_any_of(","), token_compress_on);
		}

		if (words.empty()) {
			continue;
		}

		Order::OrderTypeFlag type = (Order::OrderTypeFlag)lexical_cast<int>(words[3]);
		int volume = lexical_cast<int>(words[4]);
		double price = lexical_cast<double>(words[6]);

		string key = words[1];
		key.append("_").append(words[2]);

		ClipData* pData = nullptr;
		auto i = result.find(key);
		if (i == result.end()) {
			pData = new ClipData();
			result[key] = pData;
		}
		else {
			pData = i->second;
		}

		if (volume == 0) {
			continue;
		}

		if (type & Order::OTF_SELL) {
			volume = -volume;
		}

		if (pData->position * volume >= 0) {
			double money = pData->price * pData->position + price * volume;
			pData->position += volume;
			pData->price = money / pData->position;
		}
		else {
			pData->position += volume;
			pData->profit += volume * (pData->price - price);
		}
	}

	ofstream fs("./config/Record/Statistics.csv");

	if (!fs.good()) {
		Logger::Error() >> "<Statistics.csv> Save Failed!";
	}
	else {
		for (auto it = result.begin(); it != result.end(); ++it) {
			ClipData* pData = it->second;
			fs << it->first << ',' << pData->profit << ',' << pData->price << ',' << pData->position << endl;
		}
	}

	fs.close();
}