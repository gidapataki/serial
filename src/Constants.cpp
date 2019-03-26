#include "serial/Constants.h"
#include <ostream>


namespace serial {

const char* ToString(ErrorCode ec) {
	switch (ec) {
		case ErrorCode::kNone: return "NoError";
		case ErrorCode::kUnregisteredType: return "UnregisteredType";
		case ErrorCode::kInvalidEnumValue: return "InvalidEnumValue";
		case ErrorCode::kInvalidDocument: return "InvalidDocument";
		case ErrorCode::kInvalidHeader: return "InvalidHeader";
		case ErrorCode::kInvalidObjectHeader: return "InvalidObjectHeader";
		case ErrorCode::kInvalidObjectField: return "InvalidObjectField";
		case ErrorCode::kInvalidRootType: return "InvalidRootType";
		case ErrorCode::kInvalidReferenceType: return "InvalidReferenceType";
		case ErrorCode::kInvalidVariantType: return "InvalidVariantType";
		case ErrorCode::kInvalidSchema: return "InvalidSchema";
		case ErrorCode::kUnexpectedHeaderField: return "UnexpectedHeaderField";
		case ErrorCode::kUnexpectedObjectField: return "UnexpectedObjectField";
		case ErrorCode::kUnexpectedValue: return "UnexpectedValue";
		case ErrorCode::kMissingRootObject: return "MissingRootObject";
		case ErrorCode::kMissingHeaderField: return "MissingHeaderField";
		case ErrorCode::kMissingObjectField: return "MissingObjectField";
		case ErrorCode::kDuplicateObjectId: return "DuplicateObjectId";
		case ErrorCode::kUnresolvableReference: return "UnresolvableReference";
		case ErrorCode::kNullReference: return "NullReference";
		case ErrorCode::kEmptyVariant: return "EmptyVariant";
	}
	return "Unknown";
}

std::ostream& operator<<(std::ostream& os, ErrorCode ec) {
	auto str = ToString(ec);
	os << str;
	return os;
}

} // namespace serial
