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
	kInvalidDocument,
	kInvalidHeader,
	kInvalidObjectHeader,
	kInvalidObjectField,
	kUnexpectedHeaderField,
	kUnexpectedObjectField,
	kMissingRootObject,
	kMissingObjectField,
	kDuplicateObjectId,
	kUnresolvableReference,
};

} // namespace serial
