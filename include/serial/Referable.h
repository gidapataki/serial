#pragma once
#include "serial/ReferableBase.h"


namespace serial {

template<typename T>
class Referable : public ReferableBase {
public:
	virtual void Write(Writer* writer) const override;
	virtual void Read(Reader* reader) override;
};

} // namespace serial

#include "serial/Referable-inl.h"
