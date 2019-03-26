#pragma once
#include <cassert>
#include "serial/MetaHelpers.h"


namespace serial {
namespace detail {

struct WriteVisitor : Visitor<> {
	WriteVisitor(Writer* writer)
		: writer_(writer)
	{}

	template<typename T>
	void operator()(const T& value, const MinVersion& v0, const MaxVersion& v1) const {
		if (writer_->IsVersionInRange(v0, v1)) {
			writer_->WriteVariant(value);
		} else {
			writer_->SetError(ErrorCode::kInvalidVariantType);
		}
	}

private:
	Writer* writer_;
};

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

template<typename VisitorT, typename VariantT, typename T>
typename VisitorT::ResultType VersionedInvoker(
	VisitorT&& visitor, VariantT variant)
{
	using Info = VersionedTypeInfo<T>;
	return std::forward<VisitorT>(visitor)(
		variant.template Get<typename Info::Type>(), Info::Min(), Info::Max());
}

} // namespace detail


// Variant::ForEachType

template<typename... Ts>
template<typename U>
struct Variant<Ts...>::ForEachType<U> {
	static bool ReadIf(Variant<Ts...>& variant, TypeId id, Reader* reader) {
		using Info = VersionedTypeInfo<U>;
		using Type = typename Info::Type;

		if (StaticTypeId<Type>::Get() == id) {
			if (!reader->IsVersionInRange(Info::Min(), Info::Max())) {
				return false;
			}

			variant = Type{};
			reader->ReadVariant(variant.Get<Type>());
			return true;
		}
		return false;
	}
};

template<typename... Ts>
template<typename U, typename... Us>
struct Variant<Ts...>::ForEachType<U, Us...> {
	static bool ReadIf(Variant<Ts...>& variant, TypeId id, Reader* reader) {
		return ForEachType<U>::ReadIf(variant, id, reader) ||
			ForEachType<Us...>::ReadIf(variant, id, reader);
	}
};


// Variant

template<typename... Ts>
template<typename T, typename>
Variant<Ts...>::Variant(T&& value)
	: value_(std::forward<T>(value))
{}

template<typename... Ts>
template<typename T, typename>
Variant<Ts...>& Variant<Ts...>::operator=(T&& value) {
	value_ = std::forward<T>(value);
	return *this;
}

template<typename... Ts>
void Variant<Ts...>::Clear() {
	value_.Clear();
}

template<typename... Ts>
bool Variant<Ts...>::IsEmpty() const {
	return value_.IsEmpty();
}

template<typename... Ts>
typename Variant<Ts...>::Index Variant<Ts...>::Which() const {
	return Index(value_.Which());
}

template<typename... Ts>
template<typename T>
bool Variant<Ts...>::Is() const {
	return value_.template Is<T>();
}

template<typename... Ts>
template<typename T>
const T& Variant<Ts...>::Get() const {
	return value_.template Get<T>();
}

template<typename... Ts>
template<typename T>
T& Variant<Ts...>::Get() {
	return value_.template Get<T>();
}

template<typename... Ts>
void Variant<Ts...>::Write(Writer* writer) const {
	assert(!value_.IsEmpty() && "Cannot write empty variant");
	ApplyVersionedVisitor(detail::WriteVisitor{writer});
}

template<typename... Ts>
void Variant<Ts...>::Read(TypeId id, Reader* reader) {
	auto success = ForEachType<Ts...>::ReadIf(*this, id, reader);
	if (!success) {
		reader->SetError(ErrorCode::kInvalidVariantType);
	}
}

template<typename... Ts>
template<typename T, typename>
constexpr typename Variant<Ts...>::Index Variant<Ts...>::IndexOf() {
	return Index(internal::IndexOf<T, Types>::value);
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVisitor(V&& visitor) {
	return value_.ApplyVisitor(std::forward<V>(visitor));
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVisitor(V&& visitor) const {
	return value_.ApplyVisitor(std::forward<V>(visitor));
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVersionedVisitor(V&& visitor) {
	assert(!IsEmpty());

	using InvokerType = typename V::ResultType (*)(V&&, Variant<Ts...>&);

	// Emulated virtual dispatch
	static const InvokerType invokers[] = {
		&detail::VersionedInvoker<V, Variant<Ts...>&, Ts>...
	};

	auto which = int(Which());
	return invokers[which](std::forward<V>(visitor), *this);
}

template<typename... Ts>
template<typename V>
typename V::ResultType Variant<Ts...>::ApplyVersionedVisitor(V&& visitor) const {
	assert(!IsEmpty());

	using InvokerType = typename V::ResultType (*)(V&&, const Variant<Ts...>&);

	// Emulated virtual dispatch
	static const InvokerType invokers[] = {
		&detail::VersionedInvoker<V, const Variant<Ts...>&, Ts>...
	};

	auto which = int(Which());
	return invokers[which](std::forward<V>(visitor), *this);
}


// ApplyVisitor

template<typename V, typename... Ts>
typename V::ResultType ApplyVisitor(V&& visitor, Variant<Ts...>& variant) {
	return variant.ApplyVisitor(std::forward<V>(visitor));
}

template<typename V, typename... Ts>
typename V::ResultType ApplyVisitor(V&& visitor, const Variant<Ts...>& variant) {
	return variant.ApplyVisitor(std::forward<V>(visitor));
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

