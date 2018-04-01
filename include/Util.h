#if !defined(UTIL_H)
#define UTIL_H

#include <string>
#include <vector>
#include <fstream>

namespace Util {
	bool readFile(const std::string &fileName, std::string& out);
}


#endif
