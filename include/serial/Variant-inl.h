#pragma once
#include <cassert>

namespace serial {
namespace detail {

struct WriteVisitor : serial::internal::Visitor<> {
	WriteVisitor(Writer* writer)
		: writer_(writer)
	{}

	template<typename T>
	void operator()(const T& value) const {
		writer_->WriteVariant(value);
	}

private:
	Writer* writer_;
};

} // namespace detail


// Variant::ForEachType

template<typename... Ts>
template<typename U>
struct Variant<Ts...>::ForEachType<U> {
	static bool ReadIf(Variant<Ts...>& variant, TypeId id, Reader* reader) {
		if (StaticTypeId<U>::Get() == id) {
			variant = U{};
			reader->ReadVariant(variant.Get<U>());
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
Variant<Ts...>::Variant(Variant&& other)
	: value_(other.value_)
{}

template<typename... Ts>
template<typename T>
Variant<Ts...>::Variant(T&& value)
	: value_(value)
{}

template<typename... Ts>
Variant<Ts...>& Variant<Ts...>::operator=(const Variant& other) {
	value_ = other.value_;
	return *this;
}

template<typename... Ts>
Variant<Ts...>& Variant<Ts...>::operator=(Variant&& other) {
	value_ = other.value_;
	return *this;
}

template<typename... Ts>
template<typename T>
Variant<Ts...>& Variant<Ts...>::operator=(T&& value) {
	value_ = value;
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
	ApplyVisitor(detail::WriteVisitor{writer}, value_);
}

template<typename... Ts>
void Variant<Ts...>::Read(TypeId id, Reader* reader) {
	ForEachType<Ts...>::ReadIf(*this, id, reader);
}

template<typename... Ts>
template<typename T, typename>
constexpr typename Variant<Ts...>::Index Variant<Ts...>::IndexOf() {
	return Index(internal::IndexOf<T, internal::Typelist<Ts...>>::value);
}

} // namespace serial

