#include <nrr/util/string.h>

#include <iostream>
#include <sstream>

StringUtil::Separators StringUtil::Whitespace = { ' ', '\t', '\n', '\r' };

std::vector<std::string> StringUtil::split(const std::string &s, const StringUtil::Separators &sep) {
	std::vector<std::string> tokens;
	int bufferStart = 0;
	int bufferSize = 0;

	for (int i = 0; i < s.size(); ++i) {
		char c = s[i];
		if (std::find(sep.begin(), sep.end(), c) != sep.end()) {
			if (bufferSize == 0) {
				++bufferStart;
				continue;
			}
			tokens.push_back(s.substr(bufferStart, bufferSize));
			bufferSize = 0;
			bufferStart = i + 1;
		} else {
			++bufferSize;
		}
	}
	if (bufferSize > 0) {
		tokens.push_back(s.substr(bufferStart, bufferSize));
	}

	return tokens;
}