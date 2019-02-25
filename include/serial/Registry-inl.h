#pragma once
#include <cassert>
#include <type_traits>


namespace serial {

// Factory

template<typename T>
UniqueRef Factory<T>::Create() const {
	static_assert(
		std::is_base_of<ReferableBase, T>::value,
		"Type is not referable");

	return std::make_unique<T>();
}


// StaticTypeId

template<typename T>
TypeId StaticTypeId<T>::Get() {
	static int id;
	return &id;
};


// Registry

template<typename T>
bool Registry::Register(const char* name) {
	auto id = StaticTypeId<T>::Get();
	if (factories_.count(name) > 0) {
		if (enable_asserts_) {
			assert(false && "Duplicate type name");
		}
		return false;
	}
	if (types_.count(id) > 0) {
		if (enable_asserts_) {
			assert(false && "Type is already registered");
		}
		return false;
	}

	types_[id] = name;
	factories_[name] = std::make_unique<Factory<T>>();
	return true;
}

template<typename T>
const char* Registry::GetName() const {
	auto id = StaticTypeId<T>::Get();
	auto it = types_.find(id);
	if (it == types_.end()) {
		return nullptr;
	}

	return it->second;
}

} // namespace serial
