#include "serial/Ref.h"


namespace serial {

// TypedRefBase

ReferableBase* TypedRefBase::Get() {
	return ref_;
}

const ReferableBase* TypedRefBase::Get() const {
	return ref_;
}

} // namespace serial
