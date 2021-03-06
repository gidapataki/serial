#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <initializer_list>
#include <utility>
#include "serial/SerialFwd.h"
#include "serial/TypeId.h"
#include "serial/TypeTraits.h"
#include "serial/MetaHelpers.h"
#include "serial/Version.h"


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

using FactoryPtr = std::unique_ptr<FactoryBase>;


class Registrator {
public:
	Registrator(Registry& reg);

	template<typename T> bool RegisterAll();
	template<typename T> void VisitField(
		const T& value, const char* name, BeginVersion = {}, EndVersion = {});

private:
	template<typename... Ts> struct ForEachType;

	template<typename T> bool RegisterInternal(BeginVersion = {}, EndVersion = {});
	template<typename T> bool IsVisited() const;
	template<typename T> void AddVisited();

	template<typename T> void VisitValue(const T& value);
	template<typename T> void VisitValue(const T& value, PrimitiveTag);
	template<typename T> void VisitValue(const Array<T>& value, ArrayTag);
	template<typename T> void VisitValue(const Optional<T>& value, OptionalTag);
	template<typename T> void VisitValue(const T& value, ObjectTag);
	template<typename T> void VisitValue(const T& value, EnumTag);
	template<typename T> void VisitValue(const T& value, UserTag);
	template<typename T> void VisitValue(const T& value, ReferableTag);

	template<typename... Ts> void VisitValue(const Variant<Ts...>& value, VariantTag);
	template<typename... Ts> void VisitValue(const Ref<Ts...>& value, RefTag);

	Registry& reg_;
	bool success_ = true;
	int version_ = 0;
	std::unordered_set<TypeId> visited_;
};


class Registry {
public:
	Registry() = default;
	Registry(int version);
	Registry(noasserts_t);
	Registry(int version, noasserts_t);

	template<typename T> bool Register();
	template<typename T> bool RegisterAll();

	template<typename T> bool IsRegistered() const;
	UniqueRef CreateReferable(const std::string& name) const;

	template<typename T> bool EnumFromString(const std::string& name, T& value) const;
	template<typename T> const char* EnumToString(T value) const;

	TypeId FindTypeId(const std::string& name) const;
	int GetVersion() const;

private:
	static bool IsReserved(const std::string& name);

	template<typename T> bool Register(PrimitiveTag);
	template<typename T> bool Register(ReferableTag);
	template<typename T> bool Register(EnumTag);
	template<typename T> bool Register(ObjectTag);
	template<typename T> bool Register(UserTag);

	template<typename T>
	struct EnumValueCollector {
		EnumValueCollector(int version);
		void VisitEnumValue(T value, const char* name, BeginVersion = {}, EndVersion = {});

		std::vector<std::pair<int, const char*>> mapping;
		int version;
	};

	struct EnumMapping {
		std::unordered_map<int, const char*> names;
		std::unordered_map<std::string, int> values;
	};

	std::unordered_map<std::string, TypeId> names_;
	std::unordered_set<TypeId> typeids_;

	std::unordered_map<std::string, FactoryPtr> ref_factories_;
	std::unordered_map<TypeId, EnumMapping> enum_maps_;

	bool enable_asserts_ = true;
	int version_ = 0;
};

} // namespace serial

#include "serial/Registry-inl.h"
