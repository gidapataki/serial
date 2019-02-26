#pragma once
#include "serial/Writer.h"
#include "serial/Reader.h"
#include "serial/Registry.h"


namespace serial {

template<typename T>
bool ReferableBase::HasType() const {
	return GetTypeId() == StaticTypeId<T>::Get();
}

template<typename T>
void Referable<T>::Write(Writer* writer) const {
	writer->WriteReferable(static_cast<const T&>(*this));
}

template<typename T>
void Referable<T>::Read(Reader* reader) {
	reader->ReadReferable(static_cast<T&>(*this));
}

template<typename T>
TypeId Referable<T>::GetTypeId() const {
	return StaticTypeId<T>::Get();
}

} // namespace serial
