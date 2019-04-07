#pragma once

namespace serial {


template<typename V>
TypeVisitor<V>::TypeVisitor(int version, V& visitor)
	: version_(version)
	, visitor_(visitor)
{}


template<typename V>
template<typename T>
void TypeVisitor<V>::VisitType() {
	visitor_.template VisitType<T>();
}

template<typename V>
template<typename T>
bool TypeVisitor<V>::AcceptNamedType() {
	auto id = StaticTypeId<T>::Get();
	if (named_.count(id) > 0) {
		return false;
	}

	named_.insert(id);
	return true;
}

template<typename V>
template<typename T>
bool TypeVisitor<V>::AcceptInternalType() {
	auto id = StaticTypeId<T>::Get();
	if (internal_.count(id) > 0) {
		return false;
	}

	internal_.insert(id);
	return true;
}

template<typename V>
template<typename T>
void TypeVisitor<V>::VisitField(
	const T& value, const char* name, BeginVersion v0, EndVersion v1)
{
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}

	TraverseInternalType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::VisitVersionedType(BeginVersion v0, EndVersion v1) {
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}
	TraverseNamedType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseNamedType() {
	if (!AcceptNamedType<T>()) {
		return;
	}
	using Tag = typename TypeTag<T>::Type;
	TraverseNamedType<T>(Tag{});
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType() {
	if (!AcceptInternalType<T>()) {
		return;
	}
	using Tag = typename TypeTag<T>::Type;
	TraverseInternalType<T>(Tag{});
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseNamedType(PrimitiveTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(PrimitiveTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(ArrayTag) {
	TraverseInternalType<typename T::value_type>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(OptionalTag) {
	TraverseInternalType<typename T::value_type>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseNamedType(ObjectTag tag) {
	VisitType<T>();
	TraverseInternalType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(ObjectTag) {
	T elem;
	T::AcceptVisitor(elem, *this);
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseNamedType(EnumTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(EnumTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseNamedType(UserTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(UserTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseNamedType(ReferableTag) {
	VisitType<T>();

	T elem;
	T::AcceptVisitor(elem, *this);
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(RefTag) {
	using Types = typename T::VersionedTypes;
	ForEachVersionedType<Types>::AcceptVisitor(*this);
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseInternalType(VariantTag) {
	using Types = typename T::VersionedTypes;
	ForEachVersionedType<Types>::AcceptVisitor(*this);
}

template<typename T, typename V>
void VisitAllTypes(V& visitor, int version) {
	TypeVisitor<V> vs{version, visitor};
	vs.template TraverseNamedType<T>();
}

} // namespace serial
