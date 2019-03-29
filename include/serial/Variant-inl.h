#pragma once
#include <cassert>
#include "serial/MetaHelpers.h"


namespace serial {
namespace detail {

struct HashVisitor : Visitor<size_t> {
	template<typename T>
	size_t operator()(const T& t) const {
		return std::hash<T>{}(t);
	}
};

template<typename... Ts>
struct EqualityVisitor : Visitor<bool> {
	EqualityVisitor(const Variant<Ts...>& lhs)
		: lhs_(lhs)
	{}

	template<typename T>
	bool operator()(const T& rhs) {
		return lhs_.template Get<T>() == rhs;
	}

private:
	const Variant<Ts...>& lhs_;
};

struct CopyConstructIntoVisitor : Visitor<> {
	CopyConstructIntoVisitor(void* address)
		: address_(address)
	{}

	template<typename T>
	void operator()(const T& other) {
		new (address_) T(other);
	}

private:
	void* address_;
};

struct MoveConstructIntoVisitor : Visitor<> {
	MoveConstructIntoVisitor(void* address)
		: address_(address)
	{}

	template<typename T>
	void operator()(T& other) {
		new (address_) T(std::move(other));
	}

private:
	void* address_;
};

struct CopyIntoVisitor : Visitor<> {
	CopyIntoVisitor(void* address)
		: address_(address)
	{}

	template<typename T>
	void operator()(const T& other) {
		*reinterpret_cast<T*>(address_) = other;
	}

private:
	void* address_;
};

struct MoveIntoVisitor : Visitor<> {
	MoveIntoVisitor(void* address)
		: address_(address)
	{}

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

template<typename VisitorT, typename VariantT, typename T>
typename VisitorT::ResultType VersionedInvoker(
	VisitorT&& visitor, VariantT variant)
{
	using Info = VersionedTypeInfo<T>;
	return std::forward<VisitorT>(visitor)(
		variant.template Get<typename Info::Type>(), Info::Begin(), Info::End());
}

template<typename VisitorT, typename VariantT, typename T>
typename VisitorT::ResultType Invoker(VisitorT&& visitor, VariantT variant) {
	using Info = VersionedTypeInfo<T>;
	return std::forward<VisitorT>(visitor)(
		variant.template Get<typename Info::Type>());
}

} // namespace detail


// Variant

template<typename... Ts>
Variant<Ts...>::Variant() {}

template<typename... Ts>
Variant<Ts...>::Variant(const Variant& other) {
	if (!other.IsEmpty()) {
		other.ApplyVisitor(detail::CopyConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
}

template<typename... Ts>
Variant<Ts...>::Variant(Variant&& other) {
	if (!other.IsEmpty()) {
		other.ApplyVisitor(detail::MoveConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
}

template<typename... Ts>
Variant<Ts...>& Variant<Ts...>::operator=(const Variant& other) {
	if (other.IsEmpty()) {
		Clear();
	} else if (which_ == other.which_) {
		other.ApplyVisitor(detail::CopyIntoVisitor{GetStorage()});
	} else {
		Clear();
		other.ApplyVisitor(detail::CopyConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
	return *this;
}

template<typename... Ts>
Variant<Ts...>& Variant<Ts...>::operator=(Variant&& other) {
	if (other.IsEmpty()) {
		Clear();
	} else if (which_ == other.which_) {
		other.ApplyVisitor(detail::MoveIntoVisitor{GetStorage()});
	} else {
		Clear();
		other.ApplyVisitor(detail::MoveConstructIntoVisitor{GetStorage()});
		which_ = other.which_;
	}
	return *this;
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
template<typename T, typename>
Variant<Ts...>& Variant<Ts...>::operator=(T&& value) {
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
		ApplyVisitor(detail::DestroyerVisitor{});
		which_ = -1;
	}
}

template<typename... Ts>
bool Variant<Ts...>::IsEmpty() const {
	return which_ == -1;
}

template<typename... Ts>
typename Variant<Ts...>::Index Variant<Ts...>::Which() const {
	return Index(which_);
}

template<typename... Ts>
template<typename T>
bool Variant<Ts...>::Is() const {
	return which_ == detail::IndexOf<T, Types>::value;
}

template<typename... Ts>
template<typename T>
const T& Variant<Ts...>::Get() const {
	assert(Is<T>());
	return *reinterpret_cast<const T*>(GetStorage());
}

template<typename... Ts>
template<typename T>
T& Variant<Ts...>::Get() {
	assert(Is<T>());
	return *reinterpret_cast<T*>(GetStorage());
}

template<typename... Ts>
template<typename T, typename>
constexpr typename Variant<Ts...>::Index Variant<Ts...>::IndexOf() {
	return Index(detail::IndexOf<T, Types>::value);
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVisitor(V&& visitor) {
	assert(!IsEmpty());
	using InvokerType = typename V::ResultType (*)(V&&, Variant<Ts...>&);
	static const InvokerType invokers[] = {
		&detail::Invoker<V, Variant<Ts...>&, Ts>...
	};
	return invokers[which_](std::forward<V>(visitor), *this);
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVisitor(V&& visitor) const {
	assert(!IsEmpty());
	using InvokerType = typename V::ResultType (*)(V&&, const Variant<Ts...>&);
	static const InvokerType invokers[] = {
		&detail::Invoker<V, const Variant<Ts...>&, Ts>...
	};
	return invokers[which_](std::forward<V>(visitor), *this);
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVersionedVisitor(V&& visitor) {
	assert(!IsEmpty());
	using InvokerType = typename V::ResultType (*)(V&&, Variant<Ts...>&);
	static const InvokerType invokers[] = {
		&detail::VersionedInvoker<V, Variant<Ts...>&, Ts>...
	};
	return invokers[which_](std::forward<V>(visitor), *this);
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVersionedVisitor(V&& visitor) const {
	assert(!IsEmpty());
	using InvokerType = typename V::ResultType (*)(V&&, const Variant<Ts...>&);
	static const InvokerType invokers[] = {
		&detail::VersionedInvoker<V, const Variant<Ts...>&, Ts>...
	};
	return invokers[which_](std::forward<V>(visitor), *this);
}

template<typename... Ts>
void* Variant<Ts...>::GetStorage() {
	return reinterpret_cast<void*>(&storage_);
}

template<typename... Ts>
const void* Variant<Ts...>::GetStorage() const {
	return reinterpret_cast<const void*>(&storage_);
}

template<typename... Ts>
template<typename T>
void Variant<Ts...>::ConstructFromT(T&& value) {
	assert(IsEmpty());
	using DecayedT = typename std::decay<T>::type;
	new (GetStorage()) DecayedT(std::forward<T>(value));
	which_ = detail::IndexOf<DecayedT, Types>::value;
}


// Equality

template<typename... Ts>
bool operator==(const Variant<Ts...>& lhs, const Variant<Ts...>& rhs) {
	if (lhs.Which() != rhs.Which()) {
		return false;
	}

	if (lhs.IsEmpty()) {
		return true;
	}

	return rhs.ApplyVisitor(detail::EqualityVisitor<Ts...>{lhs});
}

template<typename... Ts>
bool operator!=(const Variant<Ts...>& lhs, const Variant<Ts...>& rhs) {
	return !(lhs == rhs);
}

} // namespace serial


namespace std {

template<typename... Ts>
typename hash<serial::Variant<Ts...>>::result_type
hash<serial::Variant<Ts...>>::operator()(const argument_type& v) const {
	size_t seed = 0;
	if (!v.IsEmpty()) {
		seed = v.ApplyVisitor(serial::detail::HashVisitor{});
	}

	// from boost::hash_combine
	auto w = int(v.Which());
	seed ^= std::hash<int>()(w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	return seed;
}


} // namespace std

