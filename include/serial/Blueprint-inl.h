#pragma once
#include <sstream>
#include <cassert>
#include "serial/TypeName.h"


namespace serial {

// Blueprint

template<typename T>
Blueprint Blueprint::FromType(int version) {
	Blueprint bp;
	BlueprintWriter w(bp, version);
	w.Add<T>();
	return bp;
}


// BlueprintWriter

template<typename T>
void BlueprintWriter::Add() {
	auto id = StaticTypeId<T>::Get();
	if (visited_.count(id) > 0) {
		return;
	}
	visited_.insert(id);

	using Tag = typename TypeTag<T>::Type;
	Add<T>(Tag{});
}

template<typename T>
void BlueprintWriter::AddTypeName(const char* info) {
	std::stringstream ss;
	ss << TypeName<T>::value << " :: " << info;
	blueprint_.AddLine(ss.str());
}

template<typename T>
void BlueprintWriter::Add(ReferableTag) {
	StateSentry sentry(this);
	PtrSet ptrs;

	state_.prefix = TypeName<T>::value;
	state_.ptrs = &ptrs;

	T elem;
	T::AcceptVisitor(elem, *this);
	AddTypeName<T>("referable");
}

template<typename T>
void BlueprintWriter::Add(EnumTag) {
	StateSentry sentry(this);
	state_.prefix = TypeName<T>::value;

	T::AcceptVisitor(*this);
	AddTypeName<T>("enum");
}

template<typename T>
void BlueprintWriter::Add(PrimitiveTag) {}

template<typename T>
void BlueprintWriter::Add(ObjectTag) {
	StateSentry sentry(this);
	PtrSet ptrs;

	state_.prefix = TypeName<T>::value;
	state_.ptrs = &ptrs;

	T elem;
	T::AcceptVisitor(elem, *this);
	AddTypeName<T>("object");
}

template<typename T>
void BlueprintWriter::Add(UserTag) {
	AddTypeName<T>("usertype");
}

template<typename T>
void BlueprintWriter::VisitField(
	const T& value, const char* name,
	BeginVersion v0, EndVersion v1)
{
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}

	assert(state_.ptrs);
	auto p = reinterpret_cast<const void*>(&value);
	if (state_.ptrs->count(p) > 0) {
		assert(false && "Field is already used");
	}
	state_.ptrs->insert(p);

	StateSentry sentry(this);
	state_.ptrs = nullptr;
	state_.prefix += '.';
	state_.prefix += name;
	VisitValue(value);
}

template<typename T>
void BlueprintWriter::VisitEnumValue(
	const T& value, const char* name,
	BeginVersion v0, EndVersion v1)
{
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}

	std::stringstream ss;
	ss << state_.prefix << " option " << name;
	blueprint_.AddLine(ss.str());
}

template<typename T>
void BlueprintWriter::VisitValue(const T& value) {
	using Tag = typename TypeTag<T>::Type;
	Tag tag;
	VisitValue(value, tag);
}

template<typename T>
void BlueprintWriter::VisitValue(const T& value, PrimitiveTag) {
	std::stringstream ss;
	ss << state_.prefix << " $ " << TypeName<T>::value;
	blueprint_.AddLine(ss.str());
}

template<typename T>
void BlueprintWriter::VisitValue(const Array<T>& value, ArrayTag) {
	StateSentry sentry(this);

	state_.prefix += "[]";
	T elem;
	VisitValue(elem);
}

template<typename T>
void BlueprintWriter::VisitValue(const Optional<T>& value, OptionalTag) {
	StateSentry sentry(this);

	state_.prefix += "?";
	T elem;
	VisitValue(elem);
}

template<typename T>
void BlueprintWriter::VisitValue(const T& value, ObjectTag) {
	StateSentry sentry(this);
	PtrSet ptrs;

	state_.ptrs = &ptrs;
	T::AcceptVisitor(value, *this);
}

template<typename T>
void BlueprintWriter::VisitValue(const T& value, EnumTag) {
	std::stringstream ss;
	ss << state_.prefix << " enum " << TypeName<T>::value;
	blueprint_.AddLine(ss.str());

	Add<T>();
}

template<typename T>
void BlueprintWriter::VisitValue(const T& value, VariantTag) {
	StateSentry sentry(this);
	state_.prefix += " variant ";
	ForEachVersionedType<typename T::VersionedTypes>::AcceptVisitor(*this);
}

template<typename T>
void BlueprintWriter::VisitVersionedType(BeginVersion v0, EndVersion v1) {
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}
	std::stringstream ss;
	ss << state_.prefix << TypeName<T>::value;
	blueprint_.AddLine(ss.str());
	Add<T>();
}

template<typename T>
void BlueprintWriter::VisitValue(const T& value, RefTag) {
	StateSentry sentry(this);
	state_.prefix += " ref ";
	ForEachVersionedType<typename T::VersionedTypes>::AcceptVisitor(*this);
}

template<typename T>
void BlueprintWriter::VisitValue(const T& value, UserTag) {
	std::stringstream ss;
	ss << state_.prefix << " user " << TypeName<T>::value;
	blueprint_.AddLine(ss.str());
	Add<T>();
}

} // namespace serial
