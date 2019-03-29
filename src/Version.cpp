#include "serial/Version.h"
#include <algorithm>


namespace serial {

BeginVersion::BeginVersion(int v)
	: value(v)
{}

EndVersion::EndVersion(int v)
	: value(v)
{}

bool IsVersionInRange(int version, const BeginVersion& v0, const EndVersion& v1) {
	return v0.value <= version && version < v1.value;
}

} // namespace serial
