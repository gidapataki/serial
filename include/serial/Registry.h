#pragma once
#include <string>
#include <unordered_map>
#include <initializer_list>
#include <utility>
#include "serial/SerialFwd.h"
#include "serial/TypeId.h"


namespace serial {

class FactoryBase {
public:
	virtual ~FactoryBase() = default;
	virtual UniqueRef Create() const = 0;
};


template<typename T>
class Factory : public FactoryBase {
public:
	virtual UniqueRef Create() const override;
};


class Registry {
public:
	template<typename T> bool Register(const char* name);
	template<typename T> const char* GetName() const;
	UniqueRef Create(const std::string& name) const;

	template<typename T> bool RegisterEnum(std::initializer_list<std::pair<T, const char*>> list);
	template<typename T> bool EnumFromString(const std::string& name, T& value) const;
	template<typename T> const char* EnumToString(T value) const;

private:
	struct EnumMapping {
		std::unordered_map<int, const char*> names;
		std::unordered_map<std::string, int> values;
	};

	std::unordered_map<TypeId, const char*> types_;
	std::unordered_map<std::string, std::unique_ptr<FactoryBase>> factories_;
	std::unordered_map<TypeId, EnumMapping> enum_maps_;

	bool enable_asserts_ = true;
};

} // namespace serial

#include "serial/Registry-inl.h"
