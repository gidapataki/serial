#include <string>
#include "serial/SerialFwd.h"


struct RgbColor : serial::UserPrimitive {
	RgbColor() = default;

	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	bool invalid = false;
};

bool FromString(const std::string& str, RgbColor& color);
bool ToString(const RgbColor& color, std::string& str);
