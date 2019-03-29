#pragma once
#include <type_traits>
#include "serial/TypeId.h"
#include "serial/TypeTraits.h"


namespace serial {

class ReferableBase;

namespace detail {

template<typename... Ts>
struct Typelist;


template<typename T>
struct Count;

template<>
struct Count<Typelist<>> {
	static constexpr int value = 0;
};

template<typename T, typename... Ts>
struct Count<Typelist<T, Ts...>> {
	static constexpr int value = 1 + Count<Typelist<Ts...>>::value;
};


template<typename T>
struct Head;

template<typename T, typename... Ts>
struct Head<Typelist<T, Ts...>> {
	using Type = T;
};


template<typename T, typename U>
struct IndexOf;

template<typename T>
struct IndexOf<T, Typelist<>> {
	static constexpr int value = -1;
};

template<typename T, typename... Ts>
struct IndexOf<T, Typelist<T, Ts...>> {
	static constexpr int value = 0;
};

template<typename T, typename U, typename... Ts>
struct IndexOf<T, Typelist<U, Ts...>> {
private:
	static constexpr int res = IndexOf<T, Typelist<Ts...>>::value;
public:
	static constexpr int value = res == -1 ? -1 : 1 + res;
};


template<typename T, typename Types>
struct IsOneOf {
	static constexpr bool value = IndexOf<T, Types>::value != -1;
};


template<typename T, typename Types>
using EnableIfOneOf = typename std::enable_if<IsOneOf<T, Types>::value>::type;

template<typename T, typename U>
using EnableIfBaseOf = typename std::enable_if<std::is_base_of<T, U>::value>::type;

template<typename T, typename Types>
using EnableIfSingle = typename std::enable_if<Count<Types>::value == 1, T>::type;


template<typename T>
struct IsAllReferable;

template<typename T>
struct IsAllReferable<Typelist<T>> {
	static constexpr bool value = std::is_base_of<ReferableBase, T>::value;
};

template<typename T, typename... Ts>
struct IsAllReferable<Typelist<T, Ts...>> {
	static constexpr bool value =
		IsAllReferable<Typelist<T>>::value &&
		IsAllReferable<Typelist<Ts...>>::value;
};


template<typename T>
struct IndexOfTypeId;

template<typename T>
struct IndexOfTypeId<Typelist<T>> {
	static int Get(TypeId id) {
		return id == StaticTypeId<T>::Get() ? 0 : -1;
	}
};

template<typename T, typename... Ts>
struct IndexOfTypeId<Typelist<T, Ts...>> {
	static int Get(TypeId id) {
		if (id == StaticTypeId<T>::Get()) {
			return 0;
		}

		auto index = IndexOfTypeId<Typelist<Ts...>>::Get(id);
		return index == -1 ? -1 : index + 1;
	}
};


template<typename T, typename Types>
struct Prepend;

template<typename T, typename... Ts>
struct Prepend<T, Typelist<Ts...>> {
	using Types = Typelist<T, Ts...>;
};

template<typename T>
struct ReturnTypeOf {
	using Type = T;
};

template<typename T, typename... Ts>
struct ReturnTypeOf<T(Ts...)> {
	using Type = T;
};

template<typename... Ts>
struct ReturnTypesOf;

template<typename T>
struct ReturnTypesOf<T> {
	using Types = Typelist<typename ReturnTypeOf<T>::Type>;
};

template<typename T, typename U, typename... Ts>
struct ReturnTypesOf<T, U, Ts...> {
	using Types = typename Prepend<
		typename ReturnTypeOf<T>::Type,
		typename ReturnTypesOf<U, Ts...>::Types>::Types;
};


template<size_t N, size_t M>
struct MaxValue {
	static constexpr size_t value = N > M ? N : M;
};


template<typename T>
struct MaxSizeOf;

template<>
struct MaxSizeOf<Typelist<>> {
	static constexpr size_t value = 0;
};

template<typename T, typename... Ts>
struct MaxSizeOf<Typelist<T, Ts...>> {
	static constexpr size_t value =
		MaxValue<sizeof(T), MaxSizeOf<Typelist<Ts...>>::value>::value;
};


template<typename T>
struct MaxAlignOf;

template<>
struct MaxAlignOf<Typelist<>> {
	static constexpr size_t value = 0;
};

template<typename T, typename... Ts>
struct MaxAlignOf<Typelist<T, Ts...>> {
	static constexpr size_t value =
		MaxValue<alignof(T), MaxAlignOf<Typelist<Ts...>>::value>::value;
};


} // namespace detail
} // namespace serial
