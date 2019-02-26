#pragma once


namespace serial {

namespace str {

constexpr const char* kDoctype = "doctype";
constexpr const char* kVersion = "version";
constexpr const char* kObjects = "objects";
constexpr const char* kType = "type";
constexpr const char* kFields = "fields";
constexpr const char* kId = "id";

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
	kMissingObjectField,
	kDuplicateObjectId,
	kUnresolvableReference,
	kNullReference,
};

} // namespace serial
