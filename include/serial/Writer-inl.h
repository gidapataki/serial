#pragma once
#include <cassert>
#include "serial/Constants.h"
#include "serial/Registry.h"
#include "serial/TypeName.h"


namespace serial {

template<typename T>
void Writer::VisitField(const T& value, const char* name, MinVersion v0, MaxVersion v1) {
	if (!InRange(v0, v1)) {
		return;
	}

	StateSentry sentry(this);
	Select(name);
	VisitValue(value);
}

template<typename T>
void Writer::WriteReferable(const T& value) {
	auto refid = AddRef(&value);
	auto name = TypeName<T>::value;

	if (!reg_.IsRegistered<T>()) {
		SetError(ErrorCode::kUnregisteredType);
		assert(!enable_asserts_ && "Type is not registered");
		return;
	}

	StateSentry sentry(this);
	Current()[str::kObjectId] = Json::Value(refid);
	Current()[str::kObjectType] = Json::Value(name);
	Select(str::kObjectFields) = Json::objectValue;
	T::AcceptVisitor(value, *this);
}

template<typename T>
void Writer::WriteVariant(const T& value) {
	if (!reg_.IsRegistered<T>()) {
		SetError(ErrorCode::kUnregisteredType);
		assert(!enable_asserts_ && "Type is not registered");
		return;
	}

	auto name = TypeName<T>::value;

	StateSentry sentry(this);
	Current()[str::kVariantType] = Json::Value(name);
	Select(str::kVariantValue) = Json::objectValue;
	VisitValue(value);
}

template<typename T>
void Writer::VisitValue(const T& value) {
	typename TypeTag<T>::Type tag;
	VisitValue(value, tag);
}

template<typename T>
void Writer::VisitValue(const T& value, RefTag) {
	if (!value) {
		SetError(ErrorCode::kNullReference);
		assert(!enable_asserts_ && "Null reference");
		return;
	}
	auto refid = AddRef(value.Get());
	Current() = Json::Value(refid);
}

template<typename T>
void Writer::VisitValue(const T& value, PrimitiveTag) {
	Current() = Json::Value(value);
}

template<typename T>
void Writer::VisitValue(const T& value, UserTag) {
	std::string str;
	bool success = value.ToString(str);
	if (!success) {
		SetError(ErrorCode::kUnexpectedValue);
		return;
	}
	Current() = str;
}

template<typename T>
void Writer::VisitValue(const T& value, ArrayTag) {
	StateSentry sentry(this);
	Current() = Json::Value(Json::arrayValue);
	for (auto& item : value) {
		StateSentry sentry2(this);
		SelectNext();
		VisitValue(item);
	}
}

template<typename T>
void Writer::VisitValue(const T& value, OptionalTag) {
	static_assert(!std::is_same<
		OptionalTag,
		typename TypeTag<typename T::value_type>::Type>::value,
		"Cannot nest Optional types");

	StateSentry sentry(this);
	if (!value) {
		Current() = Json::Value(Json::nullValue);
	} else {
		VisitValue(*value);
	}
}

template<typename T>
void Writer::VisitValue(const T& value, ObjectTag) {
	T::AcceptVisitor(value, *this);
}

template<typename T>
void Writer::VisitValue(const T& value, EnumTag) {
	auto name = reg_.EnumToString(value);
	if (name == nullptr) {
		SetError(ErrorCode::kUnregisteredEnum);
		return;
	}

	Current() = Json::Value(name);
}

template<typename T>
void Writer::VisitValue(const T& value, VariantTag) {
	if (value.IsEmpty()) {
		SetError(ErrorCode::kEmptyVariant);
	} else {
		value.Write(this);
	}
}

} // namespace serial
