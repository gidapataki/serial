#include <string>
#include "serial/SerialFwd.h"


struct RgbColor : serial::UserPrimitive {
	RgbColor() = default;

	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
};

bool FromString(const std::string& str, RgbColor& color);
void ToString(const RgbColor& color, std::string& str);
