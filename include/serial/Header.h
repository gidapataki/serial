#pragma once
#include <string>


namespace serial {

struct Header {
	Header() = default;
	Header(std::string doctype, int version)
		: doctype(std::move(doctype))
		, version(version)
	{}

	std::string doctype;
	int version = 0;
};

} // namespace serial
