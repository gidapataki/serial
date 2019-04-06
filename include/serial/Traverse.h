#pragma once
#include <unordered_set>
#include "serial/TypeId.h"
#include "serial/TypeTraits.h"
#include "serial/Version.h"


namespace serial {

template<typename V>
class TypeVisitor {
public:
	using VisitorType = V;

	TypeVisitor(int version, V& visitor);
	template<typename T> void TraverseType();

	template<typename T> void VisitField(const T& value, const char* name, BeginVersion = {}, EndVersion = {});
	template<typename T> void VisitVersionedType(BeginVersion v0, EndVersion v1);

private:
	template<typename T> void VisitType();
	template<typename T> void TraverseType(PrimitiveTag);
	template<typename T> void TraverseType(ArrayTag);
	template<typename T> void TraverseType(OptionalTag);
	template<typename T> void TraverseType(ObjectTag);
	template<typename T> void TraverseType(EnumTag);
	template<typename T> void TraverseType(UserTag);
	template<typename T> void TraverseType(ReferableTag);
	template<typename T> void TraverseType(VariantTag);
	template<typename T> void TraverseType(RefTag);

	int version_ = 0;
	VisitorType& visitor_;
	std::unordered_set<TypeId> visited_;
};


template<typename T, typename V> void VisitAllTypes(V& visitor, int version = 0);
template<typename T, typename V> void VisitAllTypes(const V& visitor, int version = 0);

} // namespace serial

#include "serial/Traverse-inl.h"
