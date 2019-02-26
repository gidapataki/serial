#include "serial/BasicRef.h"
#include "serial/TypedRef.h"


namespace serial {

// BasicRef

BasicRef::BasicRef(std::nullptr_t)
{}

BasicRef::BasicRef(ReferableBase* ref) {
	ref_ = ref;
}

BasicRef& BasicRef::operator=(std::nullptr_t) {
	ref_ = nullptr;
	return *this;
}

BasicRef& BasicRef::operator=(ReferableBase* u) {
	ref_ = u;
	return *this;
}

ReferableBase* BasicRef::Get() {
	return ref_;
}

const ReferableBase* BasicRef::Get() const {
	return ref_;
}

bool BasicRef::operator==(const BasicRef& other) const {
	return ref_ == other.ref_;
}

bool BasicRef::operator!=(const BasicRef& other) const {
	return ref_ != other.ref_;
}

BasicRef::operator bool() const {
	return ref_ != nullptr;
}


// TypedRefBase

ReferableBase* TypedRefBase::Get() {
	return ref_;
}

const ReferableBase* TypedRefBase::Get() const {
	return ref_;
}

} // namespace serial
