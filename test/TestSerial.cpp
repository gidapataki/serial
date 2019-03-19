#include "gtest/gtest.h"
#include "serial/Referable.h"
#include "serial/Serial.h"


using namespace serial;

namespace {

struct A;
struct B;

struct Color : Enum {
	enum Value : int {
		kRed,
		kOrange,
		kYellow,
	} value;

	Color() = default;
	Color(Value v) : value(v) {}

	static constexpr auto kTypeName = "color";

	template<typename V>
	static void AcceptVisitor(V& v) {
		v.VisitValue(kRed, "red");
	}
};

struct A : Referable<A> {
	int value;
	std::string name;
	Optional<Ref<A, B>> opt;

	static constexpr auto kTypeName = "a";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
		v.VisitField(self.name, "name");
		v.VisitField(self.opt, "opt");
	}
};

struct B : Referable<B> {
	Color color = {};

	static constexpr auto kTypeName = "b";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.color, "color");
	}
};

struct C : Referable<C> {
	Optional<Ref<A>> opt;

	static constexpr auto kTypeName = "c";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.opt, "opt");
	}
};

struct E : Enum {
	enum : int { kDefault } value = {};

	static constexpr auto kTypeName = "e";

	template<typename V>
	static void AcceptVisitor(V& v) {
		v.VisitValue(kDefault, nullptr);
	}
};

struct F : Referable<F> {
	using EnableAsserts = std::false_type;

	E e;

	static constexpr auto kTypeName = "f";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.e, "e");
	}
};


} // namespace

TEST(SerialTest, Serialize) {
	Json::Value root = 1;
	Header h;
	B b;
	F f;

	EXPECT_EQ(ErrorCode::kNone, Serialize(b, h, root));
	EXPECT_EQ(ErrorCode::kInvalidSchema, Serialize(f, h, root));
}

TEST(SerialTest, DeserializeHeader) {
	Json::Value root = 1;
	Header h;

	EXPECT_EQ(ErrorCode::kInvalidDocument, DeserializeHeader(root, h));
}

TEST(SerialTest, DeserializeObjects) {
	Json::Value root;
	RefContainer refs;
	Header h;
	A* a_ptr = nullptr;
	B* b_ptr = nullptr;
	C* c_ptr = nullptr;
	F* f_ptr = nullptr;

	EXPECT_EQ(ErrorCode::kInvalidDocument, DeserializeObjects(root, refs, a_ptr));

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
	root[str::kObjects][0][str::kObjectFields]["opt"] = Json::nullValue;

	refs.push_back(nullptr);
	refs.push_back(nullptr);
	EXPECT_EQ(ErrorCode::kMissingObjectField, DeserializeObjects(root, refs, a_ptr));
	EXPECT_EQ(2, refs.size());

	root[str::kObjects][0][str::kObjectFields]["value"] = 17;
	EXPECT_EQ(ErrorCode::kNone, DeserializeObjects(root, refs, a_ptr));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), a_ptr);

	EXPECT_EQ(ErrorCode::kUnregisteredType, DeserializeObjects(root, refs, b_ptr));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), a_ptr);

	EXPECT_TRUE(A::EnableAsserts::value);
	EXPECT_FALSE(F::EnableAsserts::value);

	EXPECT_EQ(ErrorCode::kInvalidRootType, DeserializeObjects(root, refs, c_ptr));
	EXPECT_EQ(ErrorCode::kInvalidSchema, DeserializeObjects(root, refs, f_ptr));

	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), a_ptr);

	root[str::kRootId] = "ref_2";
	EXPECT_EQ(ErrorCode::kMissingRootObject, DeserializeObjects(root, refs, a_ptr));
}
