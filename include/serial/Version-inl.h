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

} // namespace serial
