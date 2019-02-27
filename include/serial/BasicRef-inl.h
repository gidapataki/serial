#pragma once


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
T* BasicRef::Get() {
	if (Is<T>()) {
		return static_cast<T*>(ref_);
	}
	return nullptr;
}

template<typename T>
const T* BasicRef::Get() const {
	if (Is<T>()) {
		return static_cast<const T*>(ref_);
	}
	return nullptr;
}

} // namespace serial
