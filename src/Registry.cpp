#include "serial/Registry.h"
#include "serial/Referable.h"


namespace serial {

// Registry

Registry::Registry(noasserts_t) {
	enable_asserts_ = false;
}

UniqueRef Registry::Create(const std::string& name) const {
	auto it = factories_.find(name);
	if (it == factories_.end()) {
		return {};
	}

	return it->second->Create();
}


// Registrator

Registrator::Registrator(Registry& reg)
	: reg_(reg)
{}

} // namespace serial
