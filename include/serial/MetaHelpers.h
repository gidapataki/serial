#pragma once
#include <type_traits>
#include "serial/TypeId.h"


namespace serial {

class ReferableBase;

namespace detail {

template<typename... Ts>
struct IsOneOf;

template<typename T>
struct IsOneOf<T> {
	static const bool value = false;
};

template<typename T, typename U, typename... Ts>
struct IsOneOf<T, U, Ts...> {
	static const bool value =
		std::is_same<T, U>::value || IsOneOf<T, Ts...>::value;
};


template<typename T, typename... Ts>
using EnableIfOneOf = typename std::enable_if<IsOneOf<T, Ts...>::value>::type;

template<typename T, typename U>
using EnableIfBaseOf = typename std::enable_if<std::is_base_of<T, U>::value>::type;


template<typename... Ts>
struct IsReferable;

template<typename T>
struct IsReferable<T> {
	static const bool value = std::is_base_of<ReferableBase, T>::value;
};

template<typename T, typename... Ts>
struct IsReferable<T, Ts...> {
	static const bool value =
		IsReferable<T>::value && IsReferable<Ts...>::value;
};


template<typename... Ts>
struct MatchTypeId;

template<typename T>
struct MatchTypeId<T> {
	static bool Accept(TypeId id) {
		return id == StaticTypeId<T>::Get();
	}
};

template<typename T, typename... Ts>
struct MatchTypeId<T, Ts...> {
	static bool Accept(TypeId id) {
		return id == StaticTypeId<T>::Get() || MatchTypeId<Ts...>::Accept(id);
	}
};

} // namespace detail
} // namespace serial
