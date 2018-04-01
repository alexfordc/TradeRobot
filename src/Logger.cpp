#include "Logger.h"
#include "TimeUtil.h"


using namespace std;

Logger* Logger::m_pInstance = nullptr;
boost::mutex Logger::mMutex;

Logger& Logger::Info() {
	mMutex.lock();

	if (m_pInstance == nullptr) {
		m_pInstance = new Logger();
	}

	cout << TimeUtil::Instance().getTime() << "\tINFO:\t";

	return *m_pInstance;
}

Logger& Logger::Error() {
	mMutex.lock();

	if (m_pInstance == nullptr) {
		m_pInstance = new Logger();
	}

	cout << TimeUtil::Instance().getTime() << "\tERROR:\t";

	return *m_pInstance;
}

Logger::Logger() {
}