#pragma once

namespace serial {

using TypeId = int*;

template<typename T>
class StaticTypeId {
public:
	static TypeId Get();
};


// implementation

template<typename T>
TypeId StaticTypeId<T>::Get() {
	static int id;
	return &id;
};

} // namespace serial
