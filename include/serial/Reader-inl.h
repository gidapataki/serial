#pragma once
#include "serial/Registry.h"

namespace serial {

template<typename T>
void Reader::ReadReferable(T& value) {
	auto input_count = Current().size();
	StateSentry sentry(this);
	state_.processed = 0;

	T::AcceptVisitor(value, *this);
	if (!IsError() && state_.processed < input_count) {
		SetError(ErrorCode::kUnexpectedObjectField);
	}
}

template<typename T>
void Reader::ReadVariant(T& value) {
	StateSentry sentry(this);
	assert(Current()[str::kVariantType] == T::kTypeName);
	Select(str::kVariantValue);
	VisitValue(value);
}

template<typename T>
void Reader::VisitField(T& value, const char* name) {
	if (IsError()) {
		return;
	}

	if (!Current().isMember(name)) {
		SetError(ErrorCode::kMissingObjectField);
		return;
	}

	++state_.processed;
	StateSentry sentry(this);
	Select(name);
	VisitValue(value);
}

template<typename T>
void Reader::VisitValue(T& value) {
	typename TypeTag<T>::Type tag;
	VisitValue(value, tag);
}


template<typename T>
void Reader::VisitValue(T& value, ArrayTag) {
	if (!Current().isArray()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value.reserve(Current().size());

	for (auto& element : Current()) {
		if (IsError()) {
			return;
		}

		StateSentry sentry(this);
		Select(element);
		value.emplace_back();
		VisitValue(value.back());
	}
}

template<typename T>
void Reader::VisitValue(T& value, OptionalTag) {
	static_assert(!std::is_same<
		OptionalTag,
		typename TypeTag<typename T::value_type>::Type>::value,
		"Cannot nest Optional types");

	if (Current().isNull()) {
		value = boost::none;
	} else {
		StateSentry sentry(this);
		using ValueType = typename T::value_type;
		value = ValueType{};
		VisitValue(*value);
	}
}

template<typename T>
void Reader::VisitValue(T& value, ObjectTag) {
	if (!Current().isObject()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	auto input_count = Current().size();
	state_.processed = 0;
	T::AcceptVisitor(value, *this);
	if (!IsError() && state_.processed < input_count) {
		SetError(ErrorCode::kUnexpectedObjectField);
		return;
	}
}

template<typename T>
void Reader::VisitValue(T& value, UserTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	if (!value.FromString(Current().asString())) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}
}

template<typename T>
void Reader::VisitValue(T& value, EnumTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	if (!reg_->EnumFromString(Current().asString(), value)) {
		SetError(ErrorCode::kUnregisteredEnum);
		return;
	}
}

template<typename T>
void Reader::VisitValue(T& value, RefTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	auto refid = Current().asString();
	unresolved_refs_.emplace_back(&value, std::move(refid));
}

template<typename T>
void Reader::VisitValue(T& value, VariantTag) {
	if (!CheckVariant()) {
		return;
	}

	StateSentry sentry(this);
	auto type = Current()[str::kVariantType].asString();
	auto id = reg_->FindTypeId(type);

	if (id == kInvalidTypeId) {
		SetError(ErrorCode::kUnregisteredType);
		return;
	}
	value.Read(id, this);
}

} // namespace serial
