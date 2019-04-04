#pragma once


namespace serial {


template<int N>
BeginVersion::BeginVersion(Version<N>)
	: value(N)
{}

template<int N>
EndVersion::EndVersion(Version<N>)
	: value(N)
{}


template<typename T>
BeginVersion VersionedTypeInfo<T>::Begin() {
	return {};
}

template<typename T>
EndVersion VersionedTypeInfo<T>::End() {
	return {};
}

template<typename T, typename V>
BeginVersion VersionedTypeInfo<T(V)>::Begin() {
	return BeginVersion{V::value};
}

template<typename T, typename V>
EndVersion VersionedTypeInfo<T(V)>::End() {
	return {};
}

template<typename T, typename V, typename U>
BeginVersion VersionedTypeInfo<T(V, U)>::Begin() {
	return BeginVersion{V::value};
}

template<typename T, typename V, typename U>
EndVersion VersionedTypeInfo<T(V, U)>::End() {
	return EndVersion{U::value};
}


template<typename T>
template<typename V>
void ForEachVersionedType<detail::Typelist<T>>::AcceptVisitor(V&& visitor) {
	using Info = VersionedTypeInfo<T>;
	visitor.template VisitVersionedType<typename Info::Type>(Info::Begin(), Info::End());
}

template<typename T, typename U, typename... Ts>
template<typename V>
void ForEachVersionedType<detail::Typelist<T, U, Ts...>>::AcceptVisitor(V&& visitor) {
	ForEachVersionedType<detail::Typelist<T>>::AcceptVisitor(visitor);
	ForEachVersionedType<detail::Typelist<U, Ts...>>::AcceptVisitor(visitor);
}

} // namespace serial
