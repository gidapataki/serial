#pragma once
#include <new>
#include <cassert>
#include <type_traits>
#include "serial/MetaHelpers.h"


namespace serial {
namespace internal {

template<typename... T> using Typelist = serial::detail::Typelist<T...>;

template<typename Type, typename... T>
struct IndexOf;

template<typename T>
struct IndexOf<T, Typelist<>> {
	static constexpr int value = -1;
};

template<typename Head, typename... T>
struct IndexOf<Head, Typelist<Head, T...>> {
	static constexpr int value = 0;
};

template<typename Type, typename Head, typename... T>
struct IndexOf<Type, Typelist<Head, T...>> {
private:
	static constexpr int res = IndexOf<Type, Typelist<T...>>::value;
public:
	static constexpr int value = res == -1 ? -1 : 1 + res;
};

template<typename Type, typename List>
struct Contains {
	static constexpr bool value = IndexOf<Type, List>::value != -1;
};

/// Generic compile time Max algorithm.
/// Applies F to every T in the Typelist
/// Assumes F<T>::value is a size_t
template<typename T, template<typename> class F>
struct TypelistMax;

template<template<typename> class F>
struct TypelistMax<Typelist<>, F> {
	static_assert(sizeof(F<int>) == -1, "Can't take the maximum of zero elements");
};

template<typename H, template<typename> class F>
struct TypelistMax<Typelist<H>, F> {
	static constexpr size_t value = F<H>::value;
};

template<typename H, typename... Ts, template<typename> class F>
struct TypelistMax<Typelist<H, Ts...>, F> {
private:
	// std::max is only constexpr since C++14
	template<typename U>
	static constexpr U ConstexprMax(const U& x, const U& y) {
		return x > y ? x : y;
	}

public:
	static constexpr size_t value = ConstexprMax(F<H>::value, TypelistMax<Typelist<Ts...>, F>::value);
};

/// Expososes sizeof(T) as SizeOf<T>::value
template<typename T>
struct SizeOf {
	static constexpr size_t value = sizeof(T);
};

/// Max sizeof of all elements in a Typelist
template<typename T>
struct MaxSizeOf;

template<typename... Ts>
struct MaxSizeOf<Typelist<Ts...>> {
	static constexpr size_t value = TypelistMax<Typelist<Ts...>, SizeOf>::value;
};

/// Expososes alignof(T) as AlignOf<T>::value
template<typename T>
struct AlignOf {
	static constexpr size_t value = alignof(T);
};

/// Max alignof of all elements in a Typelist
template<typename T>
struct MaxAlignOf;

template<typename... Ts>
struct MaxAlignOf<Typelist<Ts...>> {
	static constexpr size_t value = TypelistMax<Typelist<Ts...>, AlignOf>::value;
};


template<typename T = void>
struct Visitor {
	using ResultType = T;
};

template<typename... Ts>
class Variant {
public: // Keep the public interface function body free
	using Types = Typelist<Ts...>;

	template<typename T>
	struct IndexOf {
		static constexpr int value = serial::internal::IndexOf<T, Types>::value;

		static_assert(value != -1, "Type not in typelist");
	};

	Variant();

	Variant(const Variant& other);
	Variant(Variant&& other);

	template<typename T, typename = typename std::enable_if<Contains< typename std::decay<T>::type, Types>::value>::type>
	Variant(T&& value);

	~Variant();

	Variant& operator=(const Variant& other);
	Variant& operator=(Variant&& other);

	template<typename T, typename = typename std::enable_if<Contains< typename std::decay<T>::type, Types>::value>::type>
	Variant& operator=(T&& value);

	void Clear();

	template<typename T>
	bool Is() const;

	bool IsEmpty() const;

	int Which() const;

	template<typename T>
	const T& Get() const;

	template<typename T>
	T& Get();

	template<typename Visitor>
	typename Visitor::ResultType ApplyVisitor(Visitor&& visitor);

	template<typename Visitor>
	typename Visitor::ResultType ApplyVisitor(Visitor&& visitor) const;

private:
	static const size_t kSize = MaxSizeOf<Types>::value;
	static const size_t kAlign = MaxAlignOf<Types>::value;

	using Storage = typename std::aligned_storage<kSize, kAlign>::type;

	void* GetStorage() {
		return reinterpret_cast<void*>(&storage_);
	}

	const void* GetStorage() const {
		return reinterpret_cast<const void*>(&storage_);
	}

	template<typename T>
	void ConstructFromT(T&& value) {
		assert(IsEmpty());
		using DecayedT = typename std::decay<T>::type;
		new (GetStorage()) DecayedT(std::forward<T>(value));
		which_ = IndexOf<DecayedT>::value;
	}

	// which: runtime index into Types. -1 if variant is empty
	int which_ = -1;
	Storage storage_;
};


// VariantFrom

template<typename T>
struct VariantFrom;

template<typename... Ts>
struct VariantFrom<Typelist<Ts...>> {
	using Type = Variant<Ts...>;
};


// Free function version of ApplyVisitor

template<typename Visitor, typename... Ts>
typename Visitor::ResultType ApplyVisitor(
	Visitor&& visitor,
	Variant<Ts...>& variant)
{
	return variant.ApplyVisitor(std::forward<Visitor>(visitor));
}

template<typename Visitor, typename... Ts>
typename Visitor::ResultType ApplyVisitor(
	Visitor&& visitor,
	const Variant<Ts...>& variant)
{
	return variant.ApplyVisitor(std::forward<Visitor>(visitor));
}

// --- Implementation ---

namespace detail_variant {

struct CopyConstructIntoVisitor : Visitor<> {
	CopyConstructIntoVisitor(void* address) : address_(address) {}

	template<typename T>
	void operator()(const T& other) {
		new (address_) T(other);
	}

private:
	void* address_;
};

struct MoveConstructIntoVisitor : Visitor<> {
	MoveConstructIntoVisitor(void* address) : address_(address) {}

	template<typename T>
	void operator()(T& other) {
		new (address_) T(std::move(other));
	}

private:
	void* address_;
};

struct CopyIntoVisitor : Visitor<> {
	CopyIntoVisitor(void* address) : address_(address) {}

	template<typename T>
	void operator()(const T& other) {
		*reinterpret_cast<T*>(address_) = other;
	}

private:
	void* address_;
};

struct MoveIntoVisitor : Visitor<> {
	MoveIntoVisitor(void* address) : address_(address) {}

	template<typename T>
	void operator()(T& other) {
		*reinterpret_cast<T*>(address_) = std::move(other);
	}

private:
	void* address_;
};

struct DestroyerVisitor : Visitor<> {
	template<typename T>
	void operator()(T& operand) {
		operand.~T();
	}
};

template<typename Visitor, typename Variant, typename T>
typename Visitor::ResultType VisitorInvoker(Visitor&& visitor, Variant variant) {
	return std::forward<Visitor>(visitor)(variant.template Get<T>());
}

template<typename... Ts>
struct EqualityVisitor : Visitor<bool> {
	EqualityVisitor(const Variant<Ts...>& lhs) : lhs_(lhs) {}

	template<typename T>
	bool operator()(const T& rhs) {
		return lhs_.template Get<T>() == rhs;
	}

private:
	const Variant<Ts...>& lhs_;
};

} // namespace detail_variant

template<typename... Ts> const size_t Variant<Ts...>::kSize;
template<typename... Ts> const size_t Variant<Ts...>::kAlign;

template<typename... Ts>
Variant<Ts...>::Variant() {
}

template<typename... Ts>
Variant<Ts...>::Variant(const Variant& other) {
	if (!other.IsEmpty()) {
		other.ApplyVisitor(detail_variant::CopyConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
}

template<typename... Ts>
Variant<Ts...>::Variant(Variant&& other) {
	if (!other.IsEmpty()) {
		other.ApplyVisitor(detail_variant::MoveConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
}

template<typename... Ts>
template<typename T, typename>
Variant<Ts...>::Variant(T&& value) {
	ConstructFromT(std::forward<T>(value));
}

template<typename... Ts>
Variant<Ts...>::~Variant() {
	Clear();
}

template<typename... Ts>
auto Variant<Ts...>::operator=(const Variant& other) -> Variant& {
	if (other.IsEmpty()) {
		Clear();
	} else if (which_ == other.which_) {
		other.ApplyVisitor(detail_variant::CopyIntoVisitor{GetStorage()});
	} else {
		Clear();
		other.ApplyVisitor(detail_variant::CopyConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
	return *this;
}

template<typename... Ts>
auto Variant<Ts...>::operator=(Variant&& other) -> Variant& {
	if (other.IsEmpty()) {
		Clear();
	} else if (which_ == other.which_) {
		other.ApplyVisitor(detail_variant::MoveIntoVisitor{GetStorage()});
	} else {
		Clear();
		other.ApplyVisitor(detail_variant::MoveConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
	return *this;
}

template<typename... Ts>
template<typename T, typename>
auto Variant<Ts...>::operator=(T&& value) -> Variant& {
	if (Is<typename std::decay<T>::type>()) {
		Get<typename std::decay<T>::type>() = std::forward<T>(value);
	} else {
		Clear();
		ConstructFromT(std::forward<T>(value));
	}

	return *this;
}

template<typename... Ts>
void Variant<Ts...>::Clear() {
	if (!IsEmpty()) {
		ApplyVisitor(detail_variant::DestroyerVisitor{});
		which_ = -1;
	}
}

template<typename... Ts>
template<typename T>
bool Variant<Ts...>::Is() const {
	return IndexOf<T>::value == which_;
}

template<typename... Ts>
template<typename T>
const T& Variant<Ts...>::Get() const {
	assert(Is<T>());
	return *reinterpret_cast<const T*>(GetStorage());
}

template<typename... Ts>
bool Variant<Ts...>::IsEmpty() const {
	return which_ == -1;
}

template<typename... Ts>
int Variant<Ts...>::Which() const {
	return which_;
}

template<typename... Ts>
template<typename T>
T& Variant<Ts...>::Get() {
	assert(Is<T>());
	return *reinterpret_cast<T*>(GetStorage());
}

template<typename... Ts>
template<typename Visitor>
typename Visitor::ResultType Variant<Ts...>::ApplyVisitor(Visitor&& visitor) {
	assert(!IsEmpty());

	using InvokerType = typename Visitor::ResultType (*)(Visitor&&, Variant<Ts...>&);

	// Emulated virtual dispatch
	static const InvokerType invokers[] = {
		&detail_variant::VisitorInvoker<Visitor, Variant<Ts...>&, Ts>...
	};

	return invokers[which_](std::forward<Visitor>(visitor), *this);
}

template<typename... Ts>
template<typename Visitor>
typename Visitor::ResultType Variant<Ts...>::ApplyVisitor(Visitor&& visitor) const {
	assert(!IsEmpty());

	using InvokerType = typename Visitor::ResultType (*)(Visitor&&, const Variant<Ts...>&);

	// Emulated virtual dispatch
	static const InvokerType invokers[] = {
		&detail_variant::VisitorInvoker<Visitor, const Variant<Ts...>&, Ts>...
	};

	return invokers[which_](std::forward<Visitor>(visitor), *this);
}


// Relational operators

template<typename... Ts>
bool operator==(const Variant<Ts...>& lhs, const Variant<Ts...>& rhs) {
	if (lhs.Which() != rhs.Which()) {
		return false;
	}

	if (lhs.IsEmpty()) {
		return true;
	}

	return ApplyVisitor(detail_variant::EqualityVisitor<Ts...>{lhs}, rhs);
}

template<typename... Ts>
bool operator!=(const Variant<Ts...>& lhs, const Variant<Ts...>& rhs) {
	return !(lhs == rhs);
}

} // namespace internal
} // namespace serial
