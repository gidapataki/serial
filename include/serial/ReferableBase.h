#pragma once

namespace serial {

class Reader;
class Writer;


class ReferableBase {
public:
	virtual ~ReferableBase() = default;
	virtual void Read(Reader* reader) = 0;
	virtual void Write(Writer* writer) const = 0;
};

} // namespace serial
