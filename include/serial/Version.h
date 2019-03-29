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

struct BeginVersion {
	BeginVersion() = default;
	explicit BeginVersion(int value);
	template<int N> BeginVersion(Version<N>);

	int value = 0;
};

struct EndVersion {
	EndVersion() = default;
	explicit EndVersion(int value);
	template<int N> EndVersion(Version<N>);

	int value = std::numeric_limits<int>::max();
};

bool IsVersionInRange(int version, const BeginVersion& v0, const EndVersion& v1);


template<typename T>
struct VersionedTypeInfo {
	using Type = T;

	static BeginVersion Begin();
	static EndVersion End();
};

template<typename T, typename V>
struct VersionedTypeInfo<T(V)> {
	static_assert(std::is_base_of<VersionBase, V>::value, "Invalid version type");
	using Type = T;

	static BeginVersion Begin();
	static EndVersion End();
};

template<typename T, typename V, typename U>
struct VersionedTypeInfo<T(V, U)> {
	static_assert(std::is_base_of<VersionBase, V>::value, "Invalid BeginVersion type");
	static_assert(std::is_base_of<VersionBase, U>::value, "Invalid EndVersion type");
	using Type = T;

	static BeginVersion Begin();
	static EndVersion End();
};

} // namespace serial

#include "serial/Version-inl.h"
