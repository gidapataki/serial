#include "serial/Registry.h"
#include "serial/Referable.h"


namespace serial {

UniqueRef Registry::Create(const std::string& name) const {
	auto it = factories_.find(name);
	if (it == factories_.end()) {
		return {};
	}

	return it->second->Create();
}

} // namespace serial
