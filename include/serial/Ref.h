#pragma once
#include "serial/MetaHelpers.h"


namespace serial {

class RefBase {
public:
	ReferableBase* Get();
	const ReferableBase* Get() const;
	virtual bool Set(ReferableBase* ref) = 0;

protected:
	virtual ~RefBase() {}

	ReferableBase* ref_ = nullptr;
};


template<typename... Ts>
class Ref : public RefBase {
	using FirstType = typename detail::FirstType<Ts...>::type;
public:
	enum class Index {};

	Ref() = default;
	Ref(std::nullptr_t);
	~Ref();

	Ref& operator=(std::nullptr_t);

	using RefBase::Get;

	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Ts...>> U& operator*();
	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Ts...>> const U& operator*() const;
	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Ts...>> U* operator->();
	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Ts...>> const U* operator->() const;

	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> Ref(U* u);
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> Ref& operator=(U* u);
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> U& As();
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> const U& As() const;
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> bool Is() const;
	template<typename U, typename = detail::EnableIfOneOf<U, Ts...>> static constexpr Index IndexOf();

	bool operator==(const Ref& other) const;
	bool operator!=(const Ref& other) const;

	explicit operator bool() const;
	virtual bool Set(ReferableBase* ref) override;

	Index Which() const;
};

} // namespace serial

#include "serial/Ref-inl.h"
