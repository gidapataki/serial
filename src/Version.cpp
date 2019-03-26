#include "serial/Version.h"
#include <algorithm>


namespace serial {

bool InVersionRange(const MinVersion& v0, const MaxVersion& v1, int version) {
	return v0.value <= version && v1.value > version;
}

} // namespace serial
