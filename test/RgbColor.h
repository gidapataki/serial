#include <string>
#include "serial/SerialFwd.h"


struct RgbColor : serial::UserPrimitive {
	static constexpr auto kTypeName = "color";

	bool FromString(const std::string& str);
	bool ToString(std::string& str) const;

	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	bool invalid = false;
};
