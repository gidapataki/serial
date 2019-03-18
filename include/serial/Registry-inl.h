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

#if 1
	// Note: this is faster to compile
	return UniqueRef(new T());
#else
	return std::make_unique<T>();
#endif
}


// Registry

template<typename T>
bool Registry::Register() {
	const char* name = T::kReferableName;

	if (name == nullptr) {
		assert(!enable_asserts_ && "Cannot register type with nullptr");
		return false;
	}

	auto id = StaticTypeId<T>::Get();

	if (factories_.count(name) > 0) {
		assert(!enable_asserts_ && "Duplicate type name");
		return false;
	}
	if (types_.count(id) > 0) {
		assert(!enable_asserts_ && "Type is already registered");
		return false;
	}

	types_[id] = name;
#if 1
	// Note: this is faster to compile
	factories_[name] = std::unique_ptr<FactoryBase>(new Factory<T>());
#else
	factories_[name] = std::make_unique<Factory<T>>();
#endif
	return true;
}

template<typename T>
const char* Registry::GetName() const {
	auto id = StaticTypeId<T>::Get();
	auto it = types_.find(id);
	if (it == types_.end()) {
		assert(!enable_asserts_ && "Type is not registered");
		return nullptr;
	}

	return it->second;
}

template<typename T>
bool Registry::RegisterEnum() {
	static_assert(std::is_base_of<Enum, T>::value, "Type is derived from Enum");
	auto id = StaticTypeId<T>::Get();
	if (enum_maps_.count(id) > 0) {
		assert(!enable_asserts_ && "Enum is already registered");
		return false;
	}

	T enum_value;
	EnumMapping mapping;
	EnumValueCollector<decltype(enum_value.value)> cc;
	T::AcceptVisitor(cc);

	for (auto& item : cc.mapping) {
		auto value = item.first;
		auto name = item.second;
		if (name == nullptr) {
			assert(!enable_asserts_ && "Cannot register enum with nullptr");
			return false;
		}

		if (mapping.names.count(value) > 0) {
			assert(!enable_asserts_ && "Duplicate enum value");
			return false;
		}

		if (mapping.values.count(name) > 0) {
			assert(!enable_asserts_ && "Duplicate enum value name");
			return false;
		}

		mapping.names[value] = name;
		mapping.values[name] = value;
	}

	enum_maps_[id] = std::move(mapping);
	return true;
}


template<typename T>
const char* Registry::EnumToString(T value) const {
	static_assert(std::is_base_of<Enum, T>::value, "Invalid type");

	using EnumType = decltype(value.value);
	static_assert(std::is_enum<EnumType>::value, "Type is not an enum");

	auto id = StaticTypeId<T>::Get();
	auto it = enum_maps_.find(id);
	if (it == enum_maps_.end()) {
		assert(!enable_asserts_ && "Enum is not registered");
		return nullptr;
	}

	auto& mapping = it->second;
	auto it2 = mapping.names.find(static_cast<int>(value.value));
	if (it2 == mapping.names.end()) {
		assert(!enable_asserts_ && "Enum value is not registered");
		return nullptr;
	}

	return it2->second;
}

template<typename T>
bool Registry::EnumFromString(const std::string& name, T& value) const {
	static_assert(std::is_base_of<Enum, T>::value, "Invalid type");

	using EnumType = decltype(value.value);
	static_assert(std::is_enum<EnumType>::value, "Type is not an enum");

	auto id = StaticTypeId<T>::Get();
	auto it = enum_maps_.find(id);
	if (it == enum_maps_.end()) {
		assert(!enable_asserts_ && "Enum is not registered");
		return false;
	}

	auto& mapping = it->second;
	auto it2 = mapping.values.find(name);
	if (it2 == mapping.values.end()) {
		assert(!enable_asserts_ && "Enum value name is not registered");
		return false;
	}

	value.value = static_cast<EnumType>(it2->second);
	return true;
}


template<typename T>
void Registry::EnumValueCollector<T>::VisitValue(T value, const char* name) {
	static_assert(
		std::is_same<int, typename std::underlying_type<T>::type>::value,
		"Underlying type should be int");

	mapping.emplace_back(static_cast<int>(value), name);
}

} // namespace serial
