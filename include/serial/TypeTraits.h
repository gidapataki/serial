#pragma once
#include <string>
#include <type_traits>
#include <cstdint>
#include "serial/SerialFwd.h"


namespace serial {

struct PrimitiveTag {};
struct ArrayTag {};
struct OptionalTag {};
struct ObjectTag {};
struct EnumTag {};
struct RefTag {};
struct VariantTag {};
struct ReferableTag {};
struct UserTag {};

template<typename T>
struct TypeTag {
	using Type =
		typename std::conditional<
			std::is_base_of<Enum, T>::value,
			EnumTag,
		typename std::conditional<
			std::is_base_of<UserPrimitive, T>::value,
			UserTag,
		typename std::conditional<
			std::is_base_of<ReferableBase, T>::value,
			ReferableTag,
			ObjectTag
		>::type>::type>::type;
};

template<typename T>
struct TypeTag<Array<T>> {
	using Type = ArrayTag;
};

template<typename T>
struct TypeTag<Optional<T>> {
	using Type = OptionalTag;
};

template<typename... Ts>
struct TypeTag<Ref<Ts...>> {
	using Type = RefTag;
};

template<typename... Ts>
struct TypeTag<Variant<Ts...>> {
	using Type = VariantTag;
};


// Primitives

template<> struct TypeTag<bool> { using Type = PrimitiveTag; };
template<> struct TypeTag<int32_t> { using Type = PrimitiveTag; };
template<> struct TypeTag<int64_t> { using Type = PrimitiveTag; };
template<> struct TypeTag<uint32_t> { using Type = PrimitiveTag; };
template<> struct TypeTag<uint64_t> { using Type = PrimitiveTag; };
template<> struct TypeTag<float> { using Type = PrimitiveTag; };
template<> struct TypeTag<double> { using Type = PrimitiveTag; };
template<> struct TypeTag<std::string> { using Type = PrimitiveTag; };

} // namespace serial
