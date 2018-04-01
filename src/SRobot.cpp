#include <string>
#include "Trader.h"
#include <boost/filesystem.hpp>
#include <boost/chrono/chrono.hpp>
#include "Logger.h"
#include "TimeUtil.h"
#include "Statistics.h"


using namespace std;
using namespace boost;
using namespace boost::chrono;

int main(int argc, char *argv[])
{
	if (argc != 2) {
		return 0;
	}

	if (strcmp(argv[1], "-s") == 0) {
		Logger::Info() >> "Statistics Start... ";

		Statistics& statistics = Statistics::Instance();

		statistics.run();
	}
	else {
		Trader& trader = Trader::Instance();

		if (trader.init(argv[1])) {
			Logger::Info() >> "XRobot Initialized... ";

			steady_clock::time_point time = steady_clock::now();
			while (true) {
				steady_clock::time_point now = steady_clock::now();
				long long dt = duration_cast<milliseconds>(now - time).count();
				trader.update(dt);
				time = now;
				boost::this_thread::sleep(boost::posix_time::milliseconds(500));
			}
		}
	}

	return 0;
}