#pragma once
#include <cassert>
#include "serial/Constants.h"
#include "serial/Registry.h"


namespace serial {

template<typename T>
void Writer::VisitField(const T& value, const char* name) {
	StateSentry sentry(this);
	Select(name);
	VisitValue(value);
}

template<typename T>
void Writer::WriteReferable(const T& value) {
	auto refid = refids_[&value];
	auto name = reg_.GetName<T>();

	if (name == nullptr) {
		SetError(ErrorCode::kUnregisteredType);
		assert(!enable_asserts_ && "Type is not registered");
		return;
	}

	StateSentry sentry(this);
	Current()[str::kObjectId] = Json::Value(refid);
	Current()[str::kObjectType] = Json::Value(name);
	Select(str::kObjectFields) = Json::Value(Json::objectValue);
	T::AcceptVisitor(value, *this);
}

template<typename T>
void Writer::VisitValue(const T& value) {
	typename TypeTag<T>::Type tag;
	VisitValue(value, tag);
}

template<typename T>
void Writer::VisitValue(const T& value, TypedRefTag) {
	if (!value) {
		SetError(ErrorCode::kNullReference);
		assert(!enable_asserts_ && "Null reference");
		return;
	}
	Add(value.Get());
	auto refid = refids_[value.Get()];
	Current() = Json::Value(refid);
}

template<typename T>
void Writer::VisitValue(const T& value, BasicRefTag) {
	if (!value) {
		SetError(ErrorCode::kNullReference);
		assert(!enable_asserts_ && "Null reference");
		return;
	}
	Add(value.Get());
	auto refid = refids_[value.Get()];
	Current() = Json::Value(refid);
}

template<typename T>
void Writer::VisitValue(const T& value, PrimitiveTag) {
	Current() = Json::Value(value);
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

} // namespace serial
