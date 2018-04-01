#if !defined(LOGGER_H)
#define LOGGER_H

#include <boost/thread/thread.hpp>
#include <string>


class Logger
{
public:
	static Logger& Info();

	static Logger& Error();

	template<typename T>
	Logger& operator << (const T& t) {
		std::cout << t;
		return *this;
	}

	Logger& operator << (const char* t) {
		std::cout << t;
		return *this;
	}

	template<typename T>
	void operator >> (const T& t) {
		std::cout << t << std::endl;
		mMutex.unlock();
	}

	void operator >> (const char* t) {
		std::cout << t << std::endl;
		mMutex.unlock();
	}

private:
	static Logger* m_pInstance;

	static boost::mutex mMutex;

	Logger();
};

#endif
