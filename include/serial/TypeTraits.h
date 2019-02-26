#pragma once
#include <string>
#include <type_traits>
#include "serial/SerialFwd.h"


namespace serial {

struct PrimitiveTag {};
struct ArrayTag {};
struct ObjectTag {};
struct EnumTag {};
struct BasicRefTag {};
struct TypedRefTag {};


template<typename T>
struct TypeTag {
	using Type = typename std::conditional<
		std::is_enum<T>::value,
		EnumTag,
		ObjectTag>::type;
};

template<typename T>
struct TypeTag<Array<T>> {
	using Type = ArrayTag;
};

template<>
struct TypeTag<BasicRef> {
	using Type = BasicRefTag;
};

template<typename... Ts>
struct TypeTag<TypedRef<Ts...>> {
	using Type = TypedRefTag;
};


// Primitives

template<> struct TypeTag<int> { using Type = PrimitiveTag; };
template<> struct TypeTag<std::string> { using Type = PrimitiveTag; };

} // namespace serial
