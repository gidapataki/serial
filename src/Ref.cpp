#include "serial/Ref.h"


namespace serial {

// RefBase

ReferableBase* RefBase::Get() {
	return ref_;
}

const ReferableBase* RefBase::Get() const {
	return ref_;
}

} // namespace serial
