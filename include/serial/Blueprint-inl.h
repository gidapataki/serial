#pragma once
#include <sstream>
#include "serial/TypeName.h"
#include "serial/Traverse.h"

namespace serial {

// Blueprint

template<typename T>
Blueprint Blueprint::FromType(int version) {
	Blueprint bp;
	BlueprintWriter w(bp, version);

	VisitAllTypes<T>(w, version);
	return bp;
}


// BlueprintWriter

template<typename T>
void BlueprintWriter::VisitType() {
	using Tag = typename TypeTag<T>::Type;
	VisitType<T>(Tag{});
}

template<typename T>
void BlueprintWriter::AddType(const char* info) {
	std::stringstream ss;
	ss << TypeName<T>::value << " :: " << info;
	blueprint_.AddLine(ss.str());
}

template<typename T>
void BlueprintWriter::VisitType(ReferableTag) {
	StateSentry sentry(this);
	state_.prefix = TypeName<T>::value;
	T elem;
	T::AcceptVisitor(elem, *this);
	AddType<T>("referable");
}

template<typename T>
void BlueprintWriter::VisitType(EnumTag) {
	StateSentry sentry(this);
	state_.prefix = TypeName<T>::value;

	PushQualifier("option");
	T::AcceptVisitor(*this);
	AddType<T>("enum");
}

template<typename T>
void BlueprintWriter::VisitType(PrimitiveTag) {}

template<typename T>
void BlueprintWriter::VisitType(ObjectTag) {
	StateSentry sentry(this);
	state_.prefix = TypeName<T>::value;

	T elem;
	T::AcceptVisitor(elem, *this);
	AddType<T>("object");
}

template<typename T>
void BlueprintWriter::VisitType(UserTag) {
	AddType<T>("usertype");
}

template<typename T>
void BlueprintWriter::VisitField(
	const T& value, const char* name,
	BeginVersion v0, EndVersion v1)
{
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}

	StateSentry sentry(this);
	Push(".");
	Push(name);
	VisitInternal<T>();
}

template<typename T>
void BlueprintWriter::VisitEnumValue(
	const T& value, const char* name,
	BeginVersion v0, EndVersion v1)
{
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}

	Add(name);
}

template<typename T>
void BlueprintWriter::VisitInternal() {
	using Tag = typename TypeTag<T>::Type;
	Tag tag;
	VisitInternal<T>(tag);
}

template<typename T>
void BlueprintWriter::VisitInternal(PrimitiveTag) {
	PushQualifier("$");
	Add(TypeName<T>::value);
}

template<typename T>
void BlueprintWriter::VisitInternal(ArrayTag) {
	Push("[]");
	VisitInternal<typename T::value_type>();
}

template<typename T>
void BlueprintWriter::VisitInternal(OptionalTag) {
	Push("?");
	VisitInternal<typename T::value_type>();
}

template<typename T>
void BlueprintWriter::VisitInternal(ObjectTag) {
	T elem;
	T::AcceptVisitor(elem, *this);
}

template<typename T>
void BlueprintWriter::VisitInternal(EnumTag) {
	PushQualifier("enum");
	Add(TypeName<T>::value);
}

template<typename T>
void BlueprintWriter::VisitVersionedType(BeginVersion v0, EndVersion v1) {
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}
	Add(TypeName<T>::value);
}

template<typename T>
void BlueprintWriter::VisitInternal(VariantTag) {
	PushQualifier("variant");
	ForEachVersionedType<typename T::VersionedTypes>::AcceptVisitor(*this);
}

template<typename T>
void BlueprintWriter::VisitInternal(RefTag) {
	PushQualifier("ref");
	ForEachVersionedType<typename T::VersionedTypes>::AcceptVisitor(*this);
}

template<typename T>
void BlueprintWriter::VisitInternal(UserTag) {
	PushQualifier("user");
	Add(TypeName<T>::value);
}

} // namespace serial
