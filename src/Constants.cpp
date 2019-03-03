#include "serial/Constants.h"
#include <ostream>


namespace serial {

const char* ToString(ErrorCode ec) {
	switch (ec) {
		case ErrorCode::kNone: return "NoError";
		case ErrorCode::kUnregisteredType: return "UnregisteredType";
		case ErrorCode::kUnregisteredEnum: return "UnregisteredEnum";
		case ErrorCode::kInvalidDocument: return "InvalidDocument";
		case ErrorCode::kInvalidHeader: return "InvalidHeader";
		case ErrorCode::kInvalidObjectHeader: return "InvalidObjectHeader";
		case ErrorCode::kInvalidObjectField: return "InvalidObjectField";
		case ErrorCode::kInvalidRootType: return "InvalidRootType";
		case ErrorCode::kInvalidReferenceType: return "InvalidReferenceType";
		case ErrorCode::kUnexpectedHeaderField: return "UnexpectedHeaderField";
		case ErrorCode::kUnexpectedObjectField: return "UnexpectedObjectField";
		case ErrorCode::kMissingRootObject: return "MissingRootObject";
		case ErrorCode::kMissingHeaderField: return "MissingHeaderField";
		case ErrorCode::kMissingObjectField: return "MissingObjectField";
		case ErrorCode::kDuplicateObjectId: return "DuplicateObjectId";
		case ErrorCode::kUnresolvableReference: return "UnresolvableReference";
		case ErrorCode::kNullReference: return "NullReference";
	}
	return "Unknown";
}

std::ostream& operator<<(std::ostream& os, ErrorCode ec) {
	auto str = ToString(ec);
	os << str;
	return os;
}

} // namespace serial
