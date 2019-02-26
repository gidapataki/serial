#pragma once
#include "serial/TypeId.h"

namespace serial {

class Reader;
class Writer;


class ReferableBase {
public:
	virtual ~ReferableBase() = default;
	virtual void Read(Reader* reader) = 0;
	virtual void Write(Writer* writer) const = 0;
	virtual TypeId GetTypeId() const = 0;
};

template<typename T> bool IsReferable(ReferableBase* ref);


} // namespace serial
