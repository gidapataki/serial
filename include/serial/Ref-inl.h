#pragma once
#include <cassert>
#include "serial/ReferableBase.h"
#include "serial/MetaHelpers.h"
#include "serial/Version.h"


namespace serial {

namespace detail {

template<typename T>
struct RefValidator;

template<typename T>
struct RefValidator<detail::Typelist<T>> {
	using Info = VersionedTypeInfo<T>;
	using Type = typename Info::Type;

	static bool IsSameType(TypeId id) {
		return StaticTypeId<Type>::Get() == id;
	}

	static bool IsVersionInRange(int version) {
		auto res = serial::IsVersionInRange(version, Info::Min(), Info::Max());
		return res;
	}

	static bool IsValidInVersion(int version, TypeId id) {
		return IsSameType(id) && IsVersionInRange(version);
	}
};

template<typename T, typename... Ts>
struct RefValidator<detail::Typelist<T, Ts...>> {
	static bool IsValidInVersion(int version, TypeId id) {
		using Head = RefValidator<detail::Typelist<T>>;
		using Tail = RefValidator<detail::Typelist<Ts...>>;
		if (Head::IsSameType(id)) {
			return Head::IsVersionInRange(version);
		}
		return Tail::IsValidInVersion(version, id);
	}
};

} // namespace detail


template<typename... Ts>
Ref<Ts...>::~Ref() {
	static_assert(detail::IsReferable<Types>::value, "Types should Referable");
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
template<typename U, typename>
U& Ref<Ts...>::operator*() {
	return *static_cast<U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
const U& Ref<Ts...>::operator*() const {
	return *static_cast<const U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
U* Ref<Ts...>::operator->() {
	return static_cast<U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
const U* Ref<Ts...>::operator->() const {
	return static_cast<const U*>(ref_);
}

template<typename... Ts>
template<typename U, typename>
constexpr typename Ref<Ts...>::Index Ref<Ts...>::IndexOf() {
	return Index(detail::IndexOf<U, Types>::value);
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
bool Ref<Ts...>::Resolve(int version, ReferableBase* ref) {
	if (ref == nullptr) {
		return false;
	}

	auto id = ref->GetTypeId();
	if (!detail::RefValidator<VersionedTypes>::IsValidInVersion(version, id)) {
		return false;
	}

	ref_ = ref;
	return true;
}

template<typename... Ts>
typename Ref<Ts...>::Index Ref<Ts...>::Which() const {
	if (ref_ == nullptr) {
		return Index(-1);
	}

	return Index(detail::IndexOfTypeId<Types>::Get(ref_->GetTypeId()));
}

template<typename... Ts>
bool Ref<Ts...>::IsValidInVersion(int version) const {
	return detail::RefValidator<VersionedTypes>::IsValidInVersion(
		version, ref_->GetTypeId());
}

} // namespace serial
