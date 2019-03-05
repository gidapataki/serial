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
	template<typename T> T& As();
	template<typename T> const T& As() const;

private:
	ReferableBase* ref_ = nullptr;
};

} // namespace serial

#include "serial/BasicRef-inl.h"
