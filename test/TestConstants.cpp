#include "gtest/gtest.h"
#include "serial/Constants.h"
#include <string>
#include <vector>
#include <set>

using namespace serial;

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
		ErrorCode::kUnexpectedHeaderField,
		ErrorCode::kUnexpectedObjectField,
		ErrorCode::kMissingRootObject,
		ErrorCode::kMissingHeaderField,
		ErrorCode::kMissingObjectField,
		ErrorCode::kDuplicateObjectId,
		ErrorCode::kUnresolvableReference,
		ErrorCode::kNullReference,
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
