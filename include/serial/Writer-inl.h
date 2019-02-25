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
		error_ = ErrorCode::kUnregisteredType;
		if (enable_asserts_) {
			assert(false && "Type is not registered");
		}
		return;
	}

	StateSentry sentry(this);
	Current()[str::kId] = Json::Value(refid);
	Current()[str::kType] = Json::Value(name);
	Select(str::kFields);
	T::AcceptVisitor(value, *this);
}


template<typename T>
void Writer::VisitValue(const T& value) {
	typename TypeTag<T>::Type tag;
	VisitValue(value, tag);
}

template<typename T>
void Writer::VisitValue(const T& value, RefTag) {
	Add(value);
	auto refid = refids_[value];
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

} // namespace serial