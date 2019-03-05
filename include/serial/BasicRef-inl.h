#pragma once
#include <cassert>


namespace serial {

template<typename T>
bool BasicRef::Is() const {
	static_assert(std::is_base_of<ReferableBase, T>::value, "Invalid type");
	if (!ref_) {
		return false;
	}
	return ref_->GetTypeId() == StaticTypeId<T>::Get();
}

template<typename T>
T& BasicRef::As() {
	static_assert(std::is_base_of<ReferableBase, T>::value, "Invalid type");
	assert(Is<T>() && "Invalid dynamic type");

	return *static_cast<T*>(ref_);
}

template<typename T>
const T& BasicRef::As() const {
	static_assert(std::is_base_of<ReferableBase, T>::value, "Invalid type");
	assert(Is<T>() && "Invalid dynamic type");

	return *static_cast<const T*>(ref_);
}

} // namespace serial
