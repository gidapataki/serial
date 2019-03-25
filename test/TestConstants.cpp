#include "gtest/gtest.h"
#include "serial/Constants.h"
#include <string>
#include <vector>
#include <set>
#include <sstream>

using namespace serial;

TEST(ConstantTest, StreamOperator) {
	std::stringstream ss;

	ss << ErrorCode::kMissingRootObject;
	EXPECT_EQ(std::string{ToString(ErrorCode::kMissingRootObject)}, ss.str());
}


TEST(ConstantTest, UniqueErrorCodes) {
	std::vector<std::string> names;
	std::set<std::string> uniq;
	int max_value = int(ErrorCode::kNone);

	for (auto ec : std::vector<ErrorCode>{
		ErrorCode::kNone,
		ErrorCode::kUnregisteredType,
		ErrorCode::kUnregisteredEnum,
		ErrorCode::kInvalidDocument,
		ErrorCode::kInvalidHeader,
		ErrorCode::kInvalidObjectHeader,
		ErrorCode::kInvalidObjectField,
		ErrorCode::kInvalidRootType,
		ErrorCode::kInvalidReferenceType,
		ErrorCode::kInvalidVariantType,
		ErrorCode::kInvalidSchema,
		ErrorCode::kUnexpectedHeaderField,
		ErrorCode::kUnexpectedObjectField,
		ErrorCode::kUnexpectedValue,
		ErrorCode::kMissingRootObject,
		ErrorCode::kMissingHeaderField,
		ErrorCode::kMissingObjectField,
		ErrorCode::kDuplicateObjectId,
		ErrorCode::kUnresolvableReference,
		ErrorCode::kNullReference,
		ErrorCode::kEmptyVariant,
	}) {
		names.push_back(ToString(ec));
		max_value = std::max(max_value, int(ec));
	}

	ErrorCode unknown = ErrorCode(int(max_value) + 1);
	names.push_back(ToString(unknown));

	for (auto& name : names) {
		EXPECT_TRUE(uniq.find(name) == uniq.end());
		if (uniq.find(name) != uniq.end()) {
			EXPECT_NE(name, name);
		}
		uniq.insert(name);
	}
}

TEST(ConstantTest, Names) {
	EXPECT_EQ(std::string{"doctype"}, str::kDocType);
	EXPECT_EQ(std::string{"version"}, str::kDocVersion);
	EXPECT_EQ(std::string{"objects"}, str::kObjects);
	EXPECT_EQ(std::string{"type"}, str::kObjectType);
	EXPECT_EQ(std::string{"fields"}, str::kObjectFields);
	EXPECT_EQ(std::string{"id"}, str::kObjectId);
	EXPECT_EQ(std::string{"root"}, str::kRootId);
}
