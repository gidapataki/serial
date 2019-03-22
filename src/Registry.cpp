#include "serial/Registry.h"
#include "serial/Referable.h"


namespace serial {

// Registry

Registry::Registry(noasserts_t) {
	enable_asserts_ = false;
}

UniqueRef Registry::CreateReferable(const std::string& name) const {
	auto it = ref_factories_.find(name);
	if (it == ref_factories_.end()) {
		return {};
	}

	return it->second->Create();
}

TypeId Registry::FindTypeId(const std::string& name) const {
	auto it = names_.find(name);
	if (it == names_.end()) {
		return kInvalidTypeId;
	}
	return it->second;
}

bool Registry::IsReserved(const std::string& name) {
	return !name.empty() && name.front() == '_' && name.back() == '_';
}

// Registrator

Registrator::Registrator(Registry& reg)
	: reg_(reg)
{}

} // namespace serial
