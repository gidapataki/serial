#pragma once
#include "serial/internal/Variant.h"
#include "serial/SerialFwd.h"

namespace serial {

template<typename... Ts>
class Variant {
public:
	Variant() = default;
	Variant(Variant&& other);
	template<typename T> Variant(T&& value);

	Variant& operator=(const Variant& other);
	Variant& operator=(Variant&& other);

	template<typename T> Variant& operator=(T&& value);

	void Clear();
	bool IsEmpty() const;
	int Which() const;

	template<typename T> bool Is() const;
	template<typename T> const T& Get() const;
	template<typename T> T& Get();

	void Write(Writer* writer) const;
	void Read(TypeId id, Reader* reader);

private:
	template<typename... Us> struct ForEachType;
	internal::Variant<Ts...> value_;
};

} // namespace serial

#include "serial/Variant-inl.h"
