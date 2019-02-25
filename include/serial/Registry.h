#pragma once
#include <string>
#include <unordered_map>
#include "serial/SerialFwd.h"


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


template<typename T>
class StaticTypeId {
public:
	static TypeId Get();
};


class Registry {
public:
	template<typename T> bool Register(const char* name);
	template<typename T> const char* GetName() const;
	UniqueRef Create(const std::string& name) const;

private:
	std::unordered_map<TypeId, const char*> types_;
	std::unordered_map<std::string, std::unique_ptr<FactoryBase>> factories_;
	bool enable_asserts_ = true;
};

} // namespace serial

#include "serial/Registry-inl.h"
