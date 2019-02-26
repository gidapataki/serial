#pragma once
#include <cstddef>
#include <type_traits>
#include "serial/TypeId.h"
#include "serial/ReferableBase.h"


namespace serial {

class ReferableBase;

class BasicRef {
public:
	BasicRef() = default;
	BasicRef(std::nullptr_t);
	BasicRef(ReferableBase* ref);

	BasicRef& operator=(std::nullptr_t);
	BasicRef& operator=(ReferableBase* u);

	ReferableBase* Get();
	const ReferableBase* Get() const;

	bool operator==(const BasicRef& other) const;
	bool operator!=(const BasicRef& other) const;
	explicit operator bool() const;

	template<typename T> bool Is() const;

private:
	ReferableBase* ref_ = nullptr;
};


template<typename T>
bool BasicRef::Is() const {
	static_assert(std::is_base_of<ReferableBase, T>::value, "Invalid type");
	if (!ref_) {
		return false;
	}
	return ref_->GetTypeId() == StaticTypeId<T>::Get();
}

} // namespace serial
