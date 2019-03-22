#pragma once
#include "serial/internal/Variant.h"
#include "serial/SerialFwd.h"
#include "serial/MetaHelpers.h"

namespace serial {

template<typename T = void>
struct Visitor {
	using ResultType = T;
};


template<typename... Ts>
class Variant {
public:
	enum class Index {};

	template<typename T, typename = detail::EnableIfOneOf<T, Ts...>>
	static constexpr Index IndexOf();

	Variant() = default;
	Variant(const Variant&) = default;
	Variant(Variant&&) = default;
	Variant& operator=(const Variant&) = default;
	Variant& operator=(Variant&&) = default;

	template<typename T> Variant(T&& value);
	template<typename T> Variant& operator=(T&& value);

	void Clear();
	bool IsEmpty() const;
	Index Which() const;

	template<typename T> bool Is() const;
	template<typename T> const T& Get() const;
	template<typename T> T& Get();

	void Write(Writer* writer) const;
	void Read(TypeId id, Reader* reader);

	template<typename V> typename V::ResultType ApplyVisitor(V&& visitor);
	template<typename V> typename V::ResultType ApplyVisitor(V&& visitor) const;

private:
	template<typename... Us> struct ForEachType;
	internal::Variant<Ts...> value_;
};

template<typename V, typename... Ts> typename V::ResultType ApplyVisitor(V&& visitor, Variant<Ts...>& variant);
template<typename V, typename... Ts> typename V::ResultType ApplyVisitor(V&& visitor, const Variant<Ts...>& variant);

template<typename... Ts> bool operator==(const Variant<Ts...>& lhs, const Variant<Ts...>& rhs);
template<typename... Ts> bool operator!=(const Variant<Ts...>& lhs, const Variant<Ts...>& rhs);

} // namespace serial


// hash

namespace std {

template<typename... Args>
struct hash<serial::Variant<Args...>> {
	using argument_type = serial::Variant<Args...>;
	using result_type = size_t;

	result_type operator()(const argument_type& v) const;
};

} // namespace std

#include "serial/Variant-inl.h"
