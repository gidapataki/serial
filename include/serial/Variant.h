#pragma once
#include "serial/SerialFwd.h"
#include "serial/MetaHelpers.h"

namespace serial {

template<typename... Ts>
class Variant {
public:
	using VersionedTypes = detail::Typelist<Ts...>;
	using Types = typename detail::ReturnTypesOf<Ts...>::Types;

	enum class Index {};
	template<typename T, typename = detail::EnableIfOneOf<T, Types>> static constexpr Index IndexOf();

	Variant();
	~Variant();
	Variant(const Variant& other);
	Variant(Variant&& other);
	Variant& operator=(const Variant& other);
	Variant& operator=(Variant&& other);

	template<typename T, typename = detail::EnableIfOneOf<typename std::decay<T>::type, Types>> Variant(T&& value);
	template<typename T, typename = detail::EnableIfOneOf<typename std::decay<T>::type, Types>> Variant& operator=(T&& value);

	void Clear();
	bool IsEmpty() const;
	Index Which() const;

	template<typename T> bool Is() const;
	template<typename T> const T& Get() const;
	template<typename T> T& Get();

	template<typename V> typename V::ResultType ApplyVisitor(V&& visitor);
	template<typename V> typename V::ResultType ApplyVisitor(V&& visitor) const;
	template<typename V> typename V::ResultType ApplyVersionedVisitor(V&& visitor);
	template<typename V> typename V::ResultType ApplyVersionedVisitor(V&& visitor) const;

private:
	static constexpr size_t kSize = detail::MaxSizeOf<Types>::value;
	static constexpr size_t kAlign = detail::MaxAlignOf<Types>::value;

	using Storage = typename std::aligned_storage<kSize, kAlign>::type;

	void* GetStorage();
	const void* GetStorage() const;
	template<typename T> void ConstructFromT(T&& value);

	Storage storage_;
	int which_ = -1;
};

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
