#include "gtest/gtest.h"
#include "serial/BasicRef.h"
#include "serial/Referable.h"
#include "serial/Serial.h"


using namespace serial;

namespace {

struct Color : Enum {
	enum Value : int {
		kRed,
		kOrange,
		kYellow,
	} value;

	Color() = default;
	Color(Value v) : value(v) {}

	template<typename V>
	static void AcceptVisitor(V& v) {
		v.VisitValue(kRed, "red");
	}
};

struct A : Referable<A> {
	int value;
	std::string name;

	static constexpr auto kReferableName = "a";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
		v.VisitField(self.name, "name");
	}
};

struct B : Referable<B> {
	Color color = {};

	static constexpr auto kReferableName = "b";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.color, "color");
	}
};

} // namespace

TEST(SerialTest, Serialize) {
	Json::Value root = 1;
	Registry reg(noasserts);
	Header h;
	B b;

	reg.Register<B>();
	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Serialize(&b, h, reg, root));
	EXPECT_TRUE(root.isInt());
	EXPECT_EQ(1, root.asInt());

	reg.RegisterEnum<Color>();
	EXPECT_EQ(ErrorCode::kNone, Serialize(&b, h, reg, root));
}

TEST(SerialTest, DeserializeHeader) {
	Json::Value root = 1;
	Header h;

	EXPECT_EQ(ErrorCode::kInvalidDocument, DeserializeHeader(root, h));
}

TEST(SerialTest, DeserializeObjects) {
	Json::Value root;
	BasicRef p;
	RefContainer refs;
	Registry reg;
	Header h;

	reg.Register<A>();

	EXPECT_EQ(ErrorCode::kInvalidDocument, DeserializeObjects(root, reg, refs, p));

	root = Json::objectValue;
	root[str::kDocType] = "";
	root[str::kDocVersion] = 1;
	root[str::kRootId] = "ref_1";
	root[str::kObjects] = Json::arrayValue;
	root[str::kObjects][0] = Json::objectValue;
	root[str::kObjects][0][str::kObjectType] = "a";
	root[str::kObjects][0][str::kObjectId] = "ref_1";
	root[str::kObjects][0][str::kObjectFields] = Json::objectValue;
	root[str::kObjects][0][str::kObjectFields]["name"] = "hello";

	refs.push_back(nullptr);
	refs.push_back(nullptr);
	EXPECT_EQ(ErrorCode::kMissingObjectField, DeserializeObjects(root, reg, refs, p));
	EXPECT_EQ(2, refs.size());

	root[str::kObjects][0][str::kObjectFields]["value"] = 17;
	EXPECT_EQ(ErrorCode::kNone, DeserializeObjects(root, reg, refs, p));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), p.Get());

	A* a_ptr = nullptr;
	B* b_ptr = nullptr;
	TypedRef<A> a_ref;
	TypedRef<B> b_ref;

	EXPECT_EQ(ErrorCode::kNone, DeserializeObjects(root, reg, refs, a_ptr));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), a_ptr);

	EXPECT_EQ(ErrorCode::kInvalidRootType, DeserializeObjects(root, reg, refs, b_ptr));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), a_ptr);

	EXPECT_EQ(ErrorCode::kNone, DeserializeObjects(root, reg, refs, a_ref));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), a_ref.Get());

	refs.push_back(nullptr);
	EXPECT_EQ(ErrorCode::kInvalidRootType, DeserializeObjects(root, reg, refs, b_ref));
	EXPECT_EQ(2, refs.size());

	root[str::kRootId] = "ref_2";
	EXPECT_EQ(ErrorCode::kMissingRootObject, DeserializeObjects(root, reg, refs, a_ref));
	EXPECT_EQ(ErrorCode::kMissingRootObject, DeserializeObjects(root, reg, refs, a_ptr));
}
