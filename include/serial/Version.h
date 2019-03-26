#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>
#include "serial/SerialFwd.h"


namespace serial {

struct VersionBase {};

template<int N>
struct Version : VersionBase {
	static constexpr int value = N;
};

struct MinVersion {
	MinVersion() = default;
	template<int N> MinVersion(Version<N>) : value(N) {}

	static MinVersion FromInt(int value);

	int value = std::numeric_limits<int>::min();
};


struct MaxVersion {
	MaxVersion() = default;
	template<int N> MaxVersion(Version<N>) : value(N) {}
	static MaxVersion FromInt(int value);

	int value = std::numeric_limits<int>::max();
};

bool IsVersionInRange(int version, const MinVersion& v0, const MaxVersion& v1);


template<typename T>
struct VersionedTypeInfo {
	using Type = T;

	static MinVersion Min() { return {}; }
	static MaxVersion Max() { return {}; }
};

template<typename T, typename V>
struct VersionedTypeInfo<T(V)> {
	static_assert(std::is_base_of<VersionBase, V>::value, "Invalid version type");
	using Type = T;

	static MinVersion Min() { return MinVersion::FromInt(V::value); }
	static MaxVersion Max() { return {}; }
};

template<typename T, typename V, typename U>
struct VersionedTypeInfo<T(V, U)> {
	static_assert(std::is_base_of<VersionBase, V>::value, "Invalid MinVersion type");
	static_assert(std::is_base_of<VersionBase, U>::value, "Invalid MaxVersion type");
	using Type = T;

	static MinVersion Min() { return MinVersion::FromInt(V::value); }
	static MaxVersion Max() { return MaxVersion::FromInt(V::value); }
};

} // namespace serial
