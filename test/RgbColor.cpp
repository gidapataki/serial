#include "RgbColor.h"
#include <sstream>
#include <iomanip>
#include <cctype>


bool RgbColor::FromString(const std::string& str) {
	if (str.size() != 7 || str[0] != '#') {
		return false;
	}

	for (int i = 1; i < 7; ++i) {
		if (!std::isxdigit(str[i])) {
			return false;
		}
	}

	// Note: no more errors expected here
	const char* start = str.data();
	char* end = nullptr;
	auto color_value = std::strtoul(start + 1, &end, 16);

	this->r = (color_value >> 16) & 0xff;
	this->g = (color_value >> 8) & 0xff;
	this->b = color_value & 0xff;

	return true;
}

bool RgbColor::ToString(std::string& str) const {
	if (invalid) {
		return false;
	}

	std::stringstream ss;
	ss << '#' << std::hex << std::setfill('0') << std::nouppercase;
	ss << std::setw(2) << int(r);
	ss << std::setw(2) << int(g);
	ss << std::setw(2) << int(b);
	str = ss.str();

	return true;
}
