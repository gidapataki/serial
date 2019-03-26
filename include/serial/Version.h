#pragma once
#include <cstdint>
#include <limits>

namespace serial {


struct MinVersion {
	MinVersion() = default;
	int value = std::numeric_limits<int>::min();
};


struct MaxVersion {
	MaxVersion() = default;
	int value = std::numeric_limits<int>::max();
};

bool InVersionRange(const MinVersion& v0, const MaxVersion& v1, int version);


struct VersionBase {};

template<int N>
struct Version : VersionBase {
	static constexpr int value = N;
	operator MinVersion() const {
		MinVersion v;
		v.value = value;
		return v;
	}

	operator MaxVersion() const {
		MaxVersion v;
		v.value = value;
		return v;
	}
};



} // namespace serial
