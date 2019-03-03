#pragma once
#include <iosfwd>

namespace serial {

namespace str {

constexpr const char* kDocType = "doctype";
constexpr const char* kDocVersion = "version";
constexpr const char* kObjects = "objects";
constexpr const char* kObjectType = "type";
constexpr const char* kObjectFields = "fields";
constexpr const char* kObjectId = "id";
constexpr const char* kRootId = "root";

} // namespace str


enum class ErrorCode {
	kNone,
	kUnregisteredType,
	kUnregisteredEnum,
	kInvalidDocument,
	kInvalidHeader,
	kInvalidObjectHeader,
	kInvalidObjectField,
	kInvalidRootType,
	kInvalidReferenceType,
	kUnexpectedHeaderField,
	kUnexpectedObjectField,
	kMissingRootObject,
	kMissingHeaderField,
	kMissingObjectField,
	kDuplicateObjectId,
	kUnresolvableReference,
	kNullReference,
};

const char* ToString(ErrorCode ec);

std::ostream& operator<<(std::ostream& os, ErrorCode ec);

} // namespace serial
