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
	using Types = detail::Typelist<Ts...>;

	enum class Index {};

	Ref() = default;
	Ref(std::nullptr_t);
	~Ref();

	Ref& operator=(std::nullptr_t);

	using RefBase::Get;

	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Types>> U& operator*();
	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Types>> const U& operator*() const;
	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Types>> U* operator->();
	template<typename U = FirstType, typename = detail::EnableIfSingle<U, Types>> const U* operator->() const;

	template<typename U, typename = detail::EnableIfOneOf<U, Types>> Ref(U* u);
	template<typename U, typename = detail::EnableIfOneOf<U, Types>> Ref& operator=(U* u);
	template<typename U, typename = detail::EnableIfOneOf<U, Types>> U& As();
	template<typename U, typename = detail::EnableIfOneOf<U, Types>> const U& As() const;
	template<typename U, typename = detail::EnableIfOneOf<U, Types>> bool Is() const;
	template<typename U, typename = detail::EnableIfOneOf<U, Types>> static constexpr Index IndexOf();

	bool operator==(const Ref& other) const;
	bool operator!=(const Ref& other) const;

	explicit operator bool() const;
	virtual bool Set(ReferableBase* ref) override;

	Index Which() const;
};

} // namespace serial

#include "serial/Ref-inl.h"
