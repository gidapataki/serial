#include "serial/Version.h"
#include <algorithm>


namespace serial {

MinVersion MinVersion::FromInt(int value) {
	MinVersion v;
	v.value = value;
	return v;
}

MaxVersion MaxVersion::FromInt(int value) {
	MaxVersion v;
	v.value = value;
	return v;
}


bool IsVersionInRange(int version, const MinVersion& v0, const MaxVersion& v1) {
	return v0.value <= version && v1.value > version;
}

} // namespace serial
