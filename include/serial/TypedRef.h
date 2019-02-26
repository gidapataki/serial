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


class TypedRefBase {
public:
	ReferableBase* Get() {
		return ref_;
	}

	const ReferableBase* Get() const {
		return ref_;
	}

	virtual bool Set(ReferableBase* ref) = 0;

protected:
	virtual ~TypedRefBase() {}

	ReferableBase* ref_ = nullptr;
};


template<typename... Ts>
class TypedRef : public TypedRefBase {
public:
	static_assert(detail::IsReferable<Ts...>::value, "Types should Referable");

	TypedRef() = default;
	TypedRef(nullptr_t) {}

	using TypedRefBase::Get;

	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>>
	TypedRef(U* u) {
		ref_ = u;
	}

	TypedRef& operator=(nullptr_t) {
		ref_ = nullptr;
		return *this;
	}

	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>>
	TypedRef& operator=(U* u) {
		ref_ = u;
		return *this;
	}

	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>>
	U* Get() {
		if (!ref_) {
			return nullptr;
		}
		if (ref_->HasType<U>()) {
			return static_cast<U*>(ref_);
		}
		return nullptr;
	}

	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>>
	const U* Get() const {
		if (!ref_) {
			return nullptr;
		}
		if (ref_->HasType<U>()) {
			return static_cast<const U*>(ref_);
		}
		return nullptr;
	}

	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>>
	bool Is() const {
		if (!ref_) {
			return false;
		}
		return ref_->HasType<U>();
	}

	bool operator==(const TypedRef& other) const {
		return ref_ == other.ref_;
	}

	bool operator!=(const TypedRef& other) const {
		return ref_ != other.ref_;
	}

	explicit operator bool() const {
		return ref_ != nullptr;
	}

	virtual bool Set(ReferableBase* ref) override {
		if (ref == nullptr ||
			detail::MatchTypeId<Ts...>::Accept(ref->GetTypeId()))
		{
			ref_ = ref;
			return true;
		}
		return false;
	}
};

} // namespace serial
