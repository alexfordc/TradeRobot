#include "Util.h"

bool Util::readFile(const std::string &fileName, std::string& out) {
	std::ifstream ifs(fileName.c_str(), std::ifstream::binary | std::ifstream::ate);

	if (!ifs.is_open()) {
		ifs.close();
		return false;
	}

	int nLen = (int)ifs.tellg();

	if (nLen == 0) {
		ifs.close();
		return false;
	}

	ifs.seekg(0, std::ios::beg);

	std::vector<char> bytes(nLen);
	ifs.read(&bytes[0], nLen);

	ifs.close();

	out.assign(&bytes[0], nLen);

	return true;
}