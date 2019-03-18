#pragma once
#include "serial/ReferableBase.h"
#include <cassert>


namespace serial {

template<typename... Ts>
Ref<Ts...>::~Ref() {
	static_assert(detail::IsReferable<Ts...>::value, "Types should Referable");
}

template<typename... Ts>
Ref<Ts...>::Ref(std::nullptr_t)
{}

template<typename... Ts>
template<typename U, typename>
Ref<Ts...>::Ref(U* u) {
	ref_ = u;
}

template<typename... Ts>
Ref<Ts...>& Ref<Ts...>::operator=(std::nullptr_t) {
	ref_ = nullptr;
	return *this;
}

template<typename... Ts>
template<typename U, typename>
Ref<Ts...>& Ref<Ts...>::operator=(U* u) {
	ref_ = u;
	return *this;
}

template<typename... Ts>
template<typename U, typename>
U& Ref<Ts...>::As() {
	assert(IsReferable<U>(ref_) && "Invalid dynamic type");
	return *static_cast<U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
const U& Ref<Ts...>::As() const {
	assert(IsReferable<U>(ref_) && "Invalid dynamic type");
	return *static_cast<const U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
bool Ref<Ts...>::Is() const {
	return IsReferable<U>(ref_);
}

template<typename... Ts>
bool Ref<Ts...>::operator==(const Ref& other) const {
	return ref_ == other.ref_;
}

template<typename... Ts>
bool Ref<Ts...>::operator!=(const Ref& other) const {
	return ref_ != other.ref_;
}

template<typename... Ts>
Ref<Ts...>::operator bool() const {
	return ref_ != nullptr;
}

template<typename... Ts>
bool Ref<Ts...>::Set(ReferableBase* ref) {
	if (ref == nullptr ||
		detail::MatchTypeId<Ts...>::Accept(ref->GetTypeId()))
	{
		ref_ = ref;
		return true;
	}
	return false;
}

} // namespace serial
