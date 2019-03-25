#pragma once
#include <type_traits>
#include "serial/TypeId.h"
#include "serial/TypeTraits.h"


namespace serial {

class ReferableBase;

namespace detail {

template<typename T, typename... Ts>
struct FirstType {
	using type = T;
};

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

template<typename U, typename... Ts>
using EnableIfSingle = typename std::enable_if<sizeof...(Ts) == 1, U>::type;


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


template<typename T, typename... Ts>
struct IndexOf;

template<typename T>
struct IndexOf<T> {
	static constexpr int value = -1;
};

template<typename T, typename... Ts>
struct IndexOf<T, T, Ts...> {
	static constexpr int value = 0;
};

template<typename T, typename U, typename... Ts>
struct IndexOf<T, U, Ts...> {
private:
	static constexpr int res = IndexOf<T, Ts...>::value;
public:
	static constexpr int value = res == -1 ? -1 : 1 + res;
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


template<typename... Ts>
struct IndexOfTypeId;

template<typename T>
struct IndexOfTypeId<T> {
	static int Get(TypeId id) {
		return id == StaticTypeId<T>::Get() ? 0 : -1;
	}
};

template<typename T, typename... Ts>
struct IndexOfTypeId<T, Ts...> {
	static int Get(TypeId id) {
		if (id == StaticTypeId<T>::Get()) {
			return 0;
		}

		auto index = IndexOfTypeId<Ts...>::Get(id);
		return index == -1 ? -1 : index + 1;
	}
};

} // namespace detail
} // namespace serial
