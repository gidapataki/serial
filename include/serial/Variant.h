#pragma once
#include "serial/internal/Variant.h"
#include "serial/SerialFwd.h"

namespace serial {
namespace detail {

struct WriteVisitor : serial::internal::Visitor<> {
	WriteVisitor(Writer* writer) : writer_(writer) {}

	template<typename T>
	void operator()(const T& value) const {
		writer_->WriteVariant(value);
	}

private:
	Writer* writer_;
};

} // namespace detail

template<typename... Ts>
class Variant {
public:
	Variant() = default;
	Variant(Variant&& other)
		: value_(other.value_)
	{}

	template<typename T>
	Variant(T&& value)
		: value_(value)
	{}

	Variant& operator=(const Variant& other) {
		value_ = other.value_;
		return *this;
	}

	Variant& operator=(Variant&& other) {
		value_ = other.value_;
		return *this;
	}

	template<typename T>
	Variant& operator=(T&& value) {
		value_ = value;
		return *this;
	}

	void Clear() { value_.Clear(); }

	bool IsEmpty() const { return value_.IsEmpty(); }
	int Which() const { return value_.Which(); }

	template<typename T> bool Is() const { return value_.template Is<T>(); }
	template<typename T> const T& Get() const { return value_.template Get<T>(); }
	template<typename T> T& Get() { return value_.template Get<T>(); }

	void Write(Writer* writer) const {
		if (value_.IsEmpty()) {
			writer->SetError(ErrorCode::kEmptyVariant);
		} else {
			ApplyVisitor(detail::WriteVisitor{writer}, value_);
		}
	}

private:
	internal::Variant<Ts...> value_;
};

} // namespace serial
