#pragma once
#include "serial/ReferableBase.h"
#include <cassert>


namespace serial {

template<typename... Ts>
TypedRef<Ts...>::~TypedRef() {
	static_assert(detail::IsReferable<Ts...>::value, "Types should Referable");
}

template<typename... Ts>
TypedRef<Ts...>::TypedRef(std::nullptr_t)
{}

template<typename... Ts>
template<typename U, typename>
TypedRef<Ts...>::TypedRef(U* u) {
	ref_ = u;
}

template<typename... Ts>
TypedRef<Ts...>& TypedRef<Ts...>::operator=(std::nullptr_t) {
	ref_ = nullptr;
	return *this;
}

template<typename... Ts>
template<typename U, typename>
TypedRef<Ts...>& TypedRef<Ts...>::operator=(U* u) {
	ref_ = u;
	return *this;
}

template<typename... Ts>
template<typename U, typename>
U& TypedRef<Ts...>::As() {
	assert(IsReferable<U>(ref_) && "Invalid dynamic type");
	return *static_cast<U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
const U& TypedRef<Ts...>::As() const {
	assert(IsReferable<U>(ref_) && "Invalid dynamic type");
	return *static_cast<const U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
bool TypedRef<Ts...>::Is() const {
	return IsReferable<U>(ref_);
}

template<typename... Ts>
bool TypedRef<Ts...>::operator==(const TypedRef& other) const {
	return ref_ == other.ref_;
}

template<typename... Ts>
bool TypedRef<Ts...>::operator!=(const TypedRef& other) const {
	return ref_ != other.ref_;
}

template<typename... Ts>
TypedRef<Ts...>::operator bool() const {
	return ref_ != nullptr;
}

template<typename... Ts>
bool TypedRef<Ts...>::Set(ReferableBase* ref) {
	if (ref == nullptr ||
		detail::MatchTypeId<Ts...>::Accept(ref->GetTypeId()))
	{
		ref_ = ref;
		return true;
	}
	return false;
}

} // namespace serial
