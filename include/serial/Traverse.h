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

	template<typename T> void TraverseNamedType();
	template<typename T> void TraverseInternalType();

	template<typename T> void VisitField(const T& value, const char* name, BeginVersion = {}, EndVersion = {});
	template<typename T> void VisitVersionedType(BeginVersion v0, EndVersion v1);

private:
	template<typename T> bool AcceptNamedType();
	template<typename T> bool AcceptInternalType();
	template<typename T> void VisitType();

	template<typename T> void TraverseNamedType(PrimitiveTag);
	template<typename T> void TraverseNamedType(ObjectTag);
	template<typename T> void TraverseNamedType(EnumTag);
	template<typename T> void TraverseNamedType(UserTag);
	template<typename T> void TraverseNamedType(ReferableTag);

	template<typename T> void TraverseInternalType(PrimitiveTag);
	template<typename T> void TraverseInternalType(ArrayTag);
	template<typename T> void TraverseInternalType(OptionalTag);
	template<typename T> void TraverseInternalType(ObjectTag);
	template<typename T> void TraverseInternalType(EnumTag);
	template<typename T> void TraverseInternalType(UserTag);
	template<typename T> void TraverseInternalType(VariantTag);
	template<typename T> void TraverseInternalType(RefTag);

	int version_ = 0;
	VisitorType& visitor_;
	std::unordered_set<TypeId> named_;
	std::unordered_set<TypeId> internal_;
};


template<typename T, typename V> void VisitAllTypes(V& visitor, int version = 0);

} // namespace serial

#include "serial/Traverse-inl.h"
