#include "RgbColor.h"
#include <sstream>
#include <iomanip>
#include <cctype>


bool FromString(const std::string& str, RgbColor& color) {
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

	color.r = (color_value >> 16) & 0xff;
	color.g = (color_value >> 8) & 0xff;
	color.b = color_value & 0xff;

	return true;
}

bool ToString(const RgbColor& color, std::string& str) {
	if (color.invalid) {
		return false;
	}

	std::stringstream ss;
	ss << '#' << std::hex << std::setfill('0') << std::nouppercase;
	ss << std::setw(2) << int(color.r);
	ss << std::setw(2) << int(color.g);
	ss << std::setw(2) << int(color.b);
	str = ss.str();

	return true;
}
