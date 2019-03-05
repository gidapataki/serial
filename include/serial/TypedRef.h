#pragma once
#include "serial/MetaHelpers.h"


namespace serial {

class TypedRefBase {
public:
	ReferableBase* Get();
	const ReferableBase* Get() const;
	virtual bool Set(ReferableBase* ref) = 0;

protected:
	virtual ~TypedRefBase() {}

	ReferableBase* ref_ = nullptr;
};


template<typename... Ts>
class TypedRef : public TypedRefBase {
public:
	TypedRef() = default;
	TypedRef(std::nullptr_t);
	~TypedRef();

	TypedRef& operator=(std::nullptr_t);

	using TypedRefBase::Get;

	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> TypedRef(U* u);
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> TypedRef& operator=(U* u);
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> U& As();
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> const U& As() const;
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> bool Is() const;

	bool operator==(const TypedRef& other) const;
	bool operator!=(const TypedRef& other) const;

	explicit operator bool() const;
	virtual bool Set(ReferableBase* ref) override;
};

} // namespace serial

#include "serial/TypedRef-inl.h"
