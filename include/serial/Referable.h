#pragma once
#include <type_traits>
#include "serial/ReferableBase.h"


namespace serial {

template<typename T>
class Referable : public ReferableBase {
public:
	using EnableAsserts = std::true_type;
	virtual void Write(Writer* writer) const override;
	virtual void Read(Reader* reader) override;
	virtual TypeId GetTypeId() const override;
};

} // namespace serial

#include "serial/Referable-inl.h"
