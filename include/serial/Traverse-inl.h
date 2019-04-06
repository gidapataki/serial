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
	auto id = StaticTypeId<T>::Get();
	if (visited_.count(id) > 0) {
		return;
	}
	visited_.insert(id);
	visitor_.template VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::VisitField(
	const T& value, const char* name, BeginVersion v0, EndVersion v1)
{
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}

	TraverseType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::VisitVersionedType(BeginVersion v0, EndVersion v1) {
	if (!IsVersionInRange(version_, v0, v1)) {
		return;
	}
	TraverseType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType() {
	using Tag = typename TypeTag<T>::Type;
	TraverseType<T>(Tag{});
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(PrimitiveTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(ArrayTag) {
	TraverseType<typename T::value_type>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(OptionalTag) {
	TraverseType<typename T::value_type>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(ObjectTag) {
	VisitType<T>();

	T elem;
	T::AcceptVisitor(elem, *this);
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(EnumTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(UserTag) {
	VisitType<T>();
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(ReferableTag) {
	VisitType<T>();

	T elem;
	T::AcceptVisitor(elem, *this);
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(RefTag) {
	using Types = typename T::VersionedTypes;
	ForEachVersionedType<Types>::AcceptVisitor(*this);
}

template<typename V>
template<typename T>
void TypeVisitor<V>::TraverseType(VariantTag) {
	using Types = typename T::VersionedTypes;
	ForEachVersionedType<Types>::AcceptVisitor(*this);
}

template<typename T, typename V>
void VisitAllTypes(const V& visitor, int version) {
	TypeVisitor<const V> vs{version, visitor};
	vs.template TraverseType<T>();
}

template<typename T, typename V>
void VisitAllTypes(V& visitor, int version) {
	TypeVisitor<V> vs{version, visitor};
	vs.template TraverseType<T>();
}

} // namespace serial
