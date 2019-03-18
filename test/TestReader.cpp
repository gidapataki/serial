#include "gtest/gtest.h"
#include "serial/TypedRef.h"
#include "serial/Referable.h"
#include "serial/Reader.h"
#include "RgbColor.h"
#include <limits>

using namespace serial;

namespace {

Json::Value MakeHeader(int root_id = 0) {
	Json::Value root;
	root = Json::Value(Json::objectValue);
	root[str::kDocType] = "test";
	root[str::kDocVersion] = 1;
	root[str::kRootId] = "ref_" + std::to_string(root_id);
	root[str::kObjects] = Json::Value(Json::arrayValue);
	return root;
}

Json::Value MakeObject(int id, const char* type) {
	Json::Value root;
	root[str::kObjectType] = type;
	root[str::kObjectFields] = Json::objectValue;
	root[str::kObjectId] = "ref_" + std::to_string(id);
	return root;
}

struct A;
struct B;
struct C;
struct D;
struct E;
struct F;
struct G;
struct Leaf;
struct Point;

struct Point {
	int x = 0;
	int y = 0;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.x, "x");
		v.VisitField(self.y, "y");
	}
};

struct Color : Enum {
	enum : int {
		kRed,
		kGreen,
		kBlue,
	} value = {};

	template<typename V>
	static void AcceptVisitor(V& v) {
		v.VisitValue(kRed, "red");
		v.VisitValue(kBlue, "blue");
		// Note: green is not registered
	}
};


struct Leaf : Referable<Leaf> {
	static constexpr auto kReferableName = "leaf";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct A : Referable<A> {
	int value = 0;

	static constexpr auto kReferableName = "a";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
	}
};

struct B : Referable<B> {
	std::string name;

	static constexpr auto kReferableName = "b";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.name, "name");
	}
};

struct C : Referable<C> {
	TypedRef<C, Leaf> ref;
	Array<TypedRef<A, Leaf>> elements;

	static constexpr auto kReferableName = "c";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.ref, "ref");
		v.VisitField(self.elements, "elements");
	}
};

struct D : Referable<D> {
	std::string name;
	int value = 0;

	static constexpr auto kReferableName = "d";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.name, "name");
		v.VisitField(self.value, "value");
	}
};

struct E : Referable<E> {
	Array<int> values;

	static constexpr auto kReferableName = "e";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.values, "values");
	}
};

struct F : Referable<F> {
	Point point;

	static constexpr auto kReferableName = "f";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.point, "point");
	}
};

struct G : Referable<G> {
	Color color = {};

	static constexpr auto kReferableName = "g";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.color, "color");
	}
};

struct U : Referable<U> {
	RgbColor color;

	static constexpr auto kReferableName = "u";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.color, "color");
	}
};

struct All : Referable<All> {
	bool b = {};
	int32_t i32 = {};
	int64_t i64 = {};
	uint32_t u32 = {};
	uint64_t u64 = {};
	float f = {};
	double d = {};
	std::string s;

	static constexpr auto kReferableName = "all";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.b, "b");
		v.VisitField(self.i32, "i32");
		v.VisitField(self.i64, "i64");
		v.VisitField(self.u32, "u32");
		v.VisitField(self.u64, "u64");
		v.VisitField(self.s, "s");
		v.VisitField(self.f, "f");
		v.VisitField(self.d, "d");
	}
};

struct Floats : Referable<Floats> {
	float f = {};
	double d = {};

	static constexpr auto kReferableName = "floats";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.f, "f");
		v.VisitField(self.d, "d");
	}
};

struct Opt : Referable<Opt> {
	Optional<int> i;
	Optional<Point> p;
	Optional<Array<int>> a;
	Optional<TypedRef<Opt>> ref;

	static constexpr auto kReferableName = "opt";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.i, "i");
		v.VisitField(self.p, "p");
		v.VisitField(self.a, "a");
		v.VisitField(self.ref, "ref");
	}
};

} // namespace

TEST(ReaderTest, ReadHeader) {
	Json::Value root;
	std::string unknown = "unknown";
	Header h{unknown, -1};

	root = Json::Value(Json::arrayValue);
	EXPECT_EQ(ErrorCode::kInvalidDocument, Reader(root).ReadHeader(h));
	EXPECT_EQ(unknown, h.doctype);
	EXPECT_EQ(-1, h.version);

	root = Json::Value(Json::objectValue);
	EXPECT_EQ(ErrorCode::kMissingHeaderField, Reader(root).ReadHeader(h));
	EXPECT_EQ(unknown, h.doctype);
	EXPECT_EQ(-1, h.version);

	root = MakeHeader();
	root[str::kDocType] = 1;
	EXPECT_EQ(ErrorCode::kInvalidHeader, Reader(root).ReadHeader(h));
	EXPECT_EQ(unknown, h.doctype);
	EXPECT_EQ(-1, h.version);

	root = MakeHeader();
	root[str::kDocVersion] = "hello";
	EXPECT_EQ(ErrorCode::kInvalidHeader, Reader(root).ReadHeader(h));
	EXPECT_EQ(unknown, h.doctype);
	EXPECT_EQ(-1, h.version);

	root = MakeHeader();
	root[str::kRootId] = Json::nullValue;
	EXPECT_EQ(ErrorCode::kInvalidHeader, Reader(root).ReadHeader(h));
	EXPECT_EQ(unknown, h.doctype);
	EXPECT_EQ(-1, h.version);

	root = MakeHeader();
	root[str::kObjects] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kInvalidHeader, Reader(root).ReadHeader(h));
	EXPECT_EQ(unknown, h.doctype);
	EXPECT_EQ(-1, h.version);

	root = MakeHeader();
	root["something"] = 12;
	EXPECT_EQ(ErrorCode::kUnexpectedHeaderField, Reader(root).ReadHeader(h));
	EXPECT_EQ(unknown, h.doctype);
	EXPECT_EQ(-1, h.version);

	root = MakeHeader();
	root[str::kDocType] = "header-test";
	root[str::kDocVersion] = 12;
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadHeader(h));
	EXPECT_EQ(std::string{"header-test"}, h.doctype);
	EXPECT_EQ(12, h.version);
}

TEST(ReaderTest, ReadObjects1) {
	Json::Value root;
	RefContainer refs;
	Leaf l1, l2;
	ReferableBase* p = &l2;
	Registry reg(noasserts);
	reg.Register<Leaf>();

	refs.push_back(nullptr);
	refs.push_back(nullptr);

	root = Json::arrayValue;
	EXPECT_EQ(ErrorCode::kInvalidDocument, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	root[str::kRootId] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kInvalidHeader, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader(23);
	EXPECT_EQ(ErrorCode::kMissingRootObject, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	EXPECT_EQ(ErrorCode::kMissingRootObject, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader(3);
	root[str::kObjects][0] = MakeObject(15, "leaf");
	EXPECT_EQ(ErrorCode::kMissingRootObject, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader(1);
	root[str::kObjects][0] = MakeObject(1, "leaf");
	root[str::kObjects][0]["id"] = Json::nullValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectHeader, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader(12);
	root[str::kObjects][0] = MakeObject(12, "leaf");
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), p);
}

TEST(ReaderTest, ReadObjects2) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<Leaf>();
	reg.Register<A>();
	reg.Register<B>();
	reg.Register<C>();
	reg.Register<D>();

	root = MakeHeader();
	root[str::kObjects][0] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kMissingHeaderField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader();
	root[str::kObjects][0] = MakeObject(0, "a");
	root[str::kObjects][0][str::kObjectId] = Json::nullValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectHeader, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader();
	root[str::kObjects][0] = MakeObject(0, "a");
	root[str::kObjects][0][str::kObjectType] = Json::nullValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectHeader, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader();
	root[str::kObjects][0] = MakeObject(0, "a");
	root[str::kObjects][0][str::kObjectFields] = Json::arrayValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectHeader, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader();
	root[str::kObjects][0] = MakeObject(0, "a");
	root[str::kObjects][0]["something"] = 32;
	EXPECT_EQ(ErrorCode::kUnexpectedHeaderField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader();
	root[str::kObjects][0] = MakeObject(0, "leaf");
	root[str::kObjects][1] = MakeObject(0, "leaf");
	EXPECT_EQ(ErrorCode::kDuplicateObjectId, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader();
	root[str::kObjects][0] = MakeObject(0, "leaf");
	root[str::kObjects][0][str::kObjectFields]["something"] = 1;
	EXPECT_EQ(ErrorCode::kUnexpectedObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader();
	root[str::kObjects][0] = MakeObject(0, "x");
	EXPECT_EQ(ErrorCode::kUnregisteredType, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = "ref_22";
	root[str::kObjects][0][str::kObjectFields]["ref"] = "ref_1";
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kUnresolvableReference, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = "ref_0";
	root[str::kObjects][0][str::kObjectFields]["ref"] = "ref_1";
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kInvalidReferenceType, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = "ref_1";
	root[str::kObjects][0][str::kObjectFields]["ref"] = "ref_22";
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kUnresolvableReference, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "d");
	root[str::kObjects][0][str::kObjectFields]["name"] = "kitty";
	EXPECT_EQ(ErrorCode::kMissingObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "d");
	root[str::kObjects][0][str::kObjectFields]["value"] = 12;
	EXPECT_EQ(ErrorCode::kMissingObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "d");
	root[str::kObjects][0][str::kObjectFields]["name"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["value"] = 5;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "d");
	root[str::kObjects][0][str::kObjectFields]["name"] = "hello";
	root[str::kObjects][0][str::kObjectFields]["value"] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	EXPECT_EQ(0, refs.size());
	EXPECT_EQ(nullptr, p);

	p = {};
	refs = RefContainer{};
	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "d");
	root[str::kObjects][0][str::kObjectFields]["name"] = "kitty";
	root[str::kObjects][0][str::kObjectFields]["value"] = 12;
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(1, refs.size());
	EXPECT_NE(nullptr, p);
	EXPECT_EQ(refs[0].get(), p);

	p = {};
	refs = RefContainer{};
	root = MakeHeader(1);
	root[str::kObjects][0] = MakeObject(2, "leaf");
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_NE(nullptr, p);
	EXPECT_TRUE(refs[0].get() == p || refs[1].get() == p);
}

TEST(ReaderTest, ReadObjectsWithArray) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<E>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "e");
	root[str::kObjects][0][str::kObjectFields]["values"] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "e");
	root[str::kObjects][0][str::kObjectFields]["values"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["values"][0] = 12;
	root[str::kObjects][0][str::kObjectFields]["values"][1] = "hello";
	root[str::kObjects][0][str::kObjectFields]["values"][2] = 23;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));
}

TEST(ReaderTest, ReadObjectsWithStruct) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<F>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "f");
	root[str::kObjects][0][str::kObjectFields]["point"] = 14;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "f");
	root[str::kObjects][0][str::kObjectFields]["point"] = Json::objectValue;
	root[str::kObjects][0][str::kObjectFields]["point"]["y"] = 12;
	EXPECT_EQ(ErrorCode::kMissingObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "f");
	root[str::kObjects][0][str::kObjectFields]["point"] = Json::objectValue;
	root[str::kObjects][0][str::kObjectFields]["point"]["x"] = 12;
	EXPECT_EQ(ErrorCode::kMissingObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "f");
	root[str::kObjects][0][str::kObjectFields]["point"] = Json::objectValue;
	root[str::kObjects][0][str::kObjectFields]["point"]["x"] = 1;
	root[str::kObjects][0][str::kObjectFields]["point"]["y"] = 2;
	root[str::kObjects][0][str::kObjectFields]["point"]["z"] = 3;
	EXPECT_EQ(ErrorCode::kUnexpectedObjectField, Reader(root).ReadObjects(reg, refs, p));

	EXPECT_TRUE(refs.empty());
	EXPECT_EQ(nullptr, p);

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "f");
	root[str::kObjects][0][str::kObjectFields]["point"] = Json::objectValue;
	root[str::kObjects][0][str::kObjectFields]["point"]["x"] = 1;
	root[str::kObjects][0][str::kObjectFields]["point"]["y"] = 2;
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), p);
}

TEST(ReaderTest, ReadObjectsWithEnum) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<Leaf>();
	reg.Register<C>();
	reg.Register<G>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "g");
	root[str::kObjects][0][str::kObjectFields]["color"] = 5;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "g");
	root[str::kObjects][0][str::kObjectFields]["color"] = "green";
	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Reader(root).ReadObjects(reg, refs, p));

	reg.RegisterEnum<Color>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "g");
	root[str::kObjects][0][str::kObjectFields]["color"] = "green";
	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "g");
	root[str::kObjects][0][str::kObjectFields]["color"] = "blue";
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(1, refs.size());
	EXPECT_EQ(refs[0].get(), p);
}

TEST(ReaderTest, ReadObjectsWithRef) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<G>();
	reg.Register<C>();
	reg.Register<Leaf>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["ref"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["ref"] = "ref_0";
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["ref"] = "ref_0";
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = "ref_1";
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_TRUE(refs[0].get() == p || refs[1].get() == p);
	EXPECT_EQ(StaticTypeId<C>::Get(), p->GetTypeId());
}

TEST(ReaderTest, ReadAllTypes) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<All>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "all");

	auto& fields = root[str::kObjects][0][str::kObjectFields];
	fields["b"] = true;
	fields["i32"] = -12;
	fields["i64"] = 1ll << 34;
	fields["u32"] = 55;
	fields["u64"] = 1ull << 36;
	fields["s"] = "hi_mom";
	fields["f"] = 0.25f;
	fields["d"] = 1.25e120;

	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_NE(nullptr, p);
	EXPECT_EQ(StaticTypeId<All>::Get(), p->GetTypeId());
	auto& all = static_cast<All&>(*p);

	EXPECT_EQ(true, all.b);
	EXPECT_EQ(-12, all.i32);
	EXPECT_EQ(1ll << 34, all.i64);
	EXPECT_EQ(55, all.u32);
	EXPECT_EQ(1ull << 36, all.u64);
	EXPECT_EQ(std::string{"hi_mom"}, all.s);
	EXPECT_EQ(0.25f, all.f);
	EXPECT_EQ(1.25e120, all.d);

	const Json::Value good_fields = fields;

	(fields = good_fields)["b"] = 5;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["i32"] = 1ll << 37;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["i64"] = false;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["u32"] = 1ull << 40;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["u64"] = "hm";
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["f"] = 3.25e200;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["f"] = Json::arrayValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["f"] = "hi";
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["d"] = "yo";
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["d"] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["d"] = std::numeric_limits<double>::infinity();
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	(fields = good_fields)["d"] = std::numeric_limits<double>::quiet_NaN();
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));
}

TEST(ReaderTest, InfAndNaN) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<Floats>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "floats");
	auto& fields = root[str::kObjects][0][str::kObjectFields];

	fields["f"] = "nan";
	fields["d"] = "nan";
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_TRUE(std::isnan(static_cast<Floats*>(p)->f));
	EXPECT_TRUE(std::isnan(static_cast<Floats*>(p)->d));

	fields["f"] = "inf";
	fields["d"] = "inf";
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_TRUE(std::isinf(static_cast<Floats*>(p)->f));
	EXPECT_TRUE(std::isinf(static_cast<Floats*>(p)->d));
	EXPECT_LT(0, static_cast<Floats*>(p)->f);
	EXPECT_LT(0, static_cast<Floats*>(p)->d);

	fields["f"] = "-inf";
	fields["d"] = "-inf";
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_TRUE(std::isinf(static_cast<Floats*>(p)->f));
	EXPECT_TRUE(std::isinf(static_cast<Floats*>(p)->d));
	EXPECT_GT(0, static_cast<Floats*>(p)->f);
	EXPECT_GT(0, static_cast<Floats*>(p)->d);
}

TEST(ReaderTest, Optional) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<Opt>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "opt");
	auto& fields = root[str::kObjects][0][str::kObjectFields];

	fields["i"] = Json::nullValue;
	fields["p"] = Json::nullValue;
	fields["a"] = Json::nullValue;
	fields["ref"] = Json::nullValue;

	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_FALSE(static_cast<Opt*>(p)->i);
	EXPECT_FALSE(static_cast<Opt*>(p)->p);
	EXPECT_FALSE(static_cast<Opt*>(p)->a);
	EXPECT_FALSE(static_cast<Opt*>(p)->ref);

	fields["i"] = 15;
	fields["p"] = Json::objectValue;
	fields["p"]["x"] = 6;
	fields["p"]["y"] = -13;
	fields["a"] = Json::arrayValue;
	fields["a"][0] = 3;
	fields["a"][1] = 5;
	fields["a"][2] = 7;
	fields["ref"] = "ref_0";

	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(15, static_cast<Opt*>(p)->i);
	EXPECT_EQ(6, static_cast<Opt*>(p)->p->x);
	EXPECT_EQ(-13, static_cast<Opt*>(p)->p->y);
	EXPECT_EQ(3, static_cast<Opt*>(p)->a->size());
	EXPECT_EQ(p, static_cast<Opt*>(p)->ref->Get());
}

TEST(ReaderTest, UserType) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);
	reg.Register<U>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "u");
	auto& fields = root[str::kObjects][0][str::kObjectFields];
	fields["color"] = "#hello_";

	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	fields["color"] = "#0102ff";
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));

	auto& color = static_cast<U&>(*p).color;
	EXPECT_EQ(1, color.r);
	EXPECT_EQ(2, color.g);
	EXPECT_EQ(255, color.b);
}
