#pragma once
#include "serial/Writer.h"
#include "serial/Reader.h"


namespace serial {

template<typename T>
void Referable<T>::Write(Writer* writer) const {
	writer->WriteReferable(static_cast<const T&>(*this));
}

template<typename T>
void Referable<T>::Read(Reader* reader) {
	reader->ReadReferable(static_cast<T&>(*this));
}

} // namespace serial
