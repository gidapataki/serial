#pragma once
#include <string>


namespace serial {

struct Header {
	std::string doctype;
	int version = 0;
};

} // namespace serial
