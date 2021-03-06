#include "gtest/gtest.h"
#include "serial/Ref.h"
#include "serial/Referable.h"
#include "serial/Reader.h"
#include "serial/Variant.h"
#include "RgbColor.h"
#include <limits>

using namespace serial;

namespace {

void Dump(const Json::Value& root) {
	Json::StreamWriterBuilder builder;
	builder.settings_["indentation"] = "  ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &std::cout);
	std::cout << std::endl;
}

Json::Value MakeHeader(int root_id = 0) {
	Json::Value root;
	root = Json::Value(Json::objectValue);
	root[str::kDocType] = "test";
	root[str::kDocVersion] = 1;
	root[str::kRootId] = "ref_" + std::to_string(root_id);
	root[str::kObjects] = Json::Value(Json::arrayValue);
	return root;
}

Json::Value MakeVariant(const char* name) {
	Json::Value root;
	root[str::kVariantType] = name;
	root[str::kVariantValue] = Json::nullValue;
	return root;
}

void SetDocVersion(Json::Value& root, int version) {
	root[str::kDocVersion] = version;
}

Json::Value& AddObject(Json::Value& root, Json::Value&& obj) {
	auto& arr = root[str::kObjects];
	auto index = arr.size();
	return arr[index] = obj;
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
struct U;
struct V;
struct Leaf;
struct Point;
struct Opt;
struct Floats;
struct All;

using Version1 = serial::Version<1>;
using Version2 = serial::Version<2>;
using Version3 = serial::Version<3>;


struct Point {
	int x = 0;
	int y = 0;

	static constexpr auto kTypeName = "point";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.x, "x");
		v.VisitField(self.y, "y");
	}
};

struct Color : Enum {
	enum Value : int {
		kRed,
		kGreen,
		kBlue,
		kYellow,
		kOrange,
	} value = {};

	Color() = default;
	Color(Value v) : value(v) {}

	static constexpr auto kTypeName = "color";

	template<typename V>
	static void AcceptVisitor(V& v) {
		// Note: green is not registered
		v.VisitEnumValue(kRed, "red");
		v.VisitEnumValue(kBlue, "blue");

		v.VisitEnumValue(kYellow, "yellow", Version1());
		v.VisitEnumValue(kOrange, "orange", Version1(), Version2());
	}
};


struct Leaf : Referable<Leaf> {
	static constexpr auto kTypeName = "leaf";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct A : Referable<A> {
	int value = 0;

	static constexpr auto kTypeName = "a";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
	}
};

struct B : Referable<B> {
	std::string name;

	static constexpr auto kTypeName = "b";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.name, "name");
	}
};

struct C : Referable<C> {
	Ref<C, Leaf> ref;
	Array<Ref<A, Leaf>> elements;

	static constexpr auto kTypeName = "c";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.ref, "ref");
		v.VisitField(self.elements, "elements");
	}
};

struct D : Referable<D> {
	std::string name;
	int value = 0;

	static constexpr auto kTypeName = "d";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.name, "name");
		v.VisitField(self.value, "value");
	}
};

struct E : Referable<E> {
	Array<int> values;

	static constexpr auto kTypeName = "e";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.values, "values");
	}
};

struct F : Referable<F> {
	Point point;

	static constexpr auto kTypeName = "f";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.point, "point");
	}
};

struct G : Referable<G> {
	Color color = {};

	static constexpr auto kTypeName = "g";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.color, "color");
	}
};

struct U : Referable<U> {
	RgbColor color;

	static constexpr auto kTypeName = "u";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.color, "color");
	}
};

struct V : Referable<V> {
	Variant<Point, Color, int32_t> v;

	static constexpr auto kTypeName = "v";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.v, "v");
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

	static constexpr auto kTypeName = "all";

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

	static constexpr auto kTypeName = "floats";

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
	Optional<Ref<Opt>> ref;

	static constexpr auto kTypeName = "opt";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.i, "i");
		v.VisitField(self.p, "p");
		v.VisitField(self.a, "a");
		v.VisitField(self.ref, "ref");
	}
};

struct Versioned : Referable<Versioned> {
	Color color;
	Variant<int, std::string(Version1), float(Version1, Version2)> v;
	Variant<int, std::string, float> w;
	Ref<Versioned, A(Version1), B(Version1, Version2)> ref;
	int i1 = -1;
	int i2 = -1;
	int i3 = -1;

	static constexpr auto kTypeName = "versioned";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.color, "c");
		v.VisitField(self.v, "v");
		v.VisitField(self.w, "w");
		v.VisitField(self.ref, "r");
		v.VisitField(self.i1, "i1", Version1());
		v.VisitField(self.i2, "i2", Version2(), Version3());
		v.VisitField(self.i3, "i3", {}, Version3());
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

	root = MakeHeader();
	root[str::kDocVersion] = Json::objectValue;
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
	EXPECT_EQ(ErrorCode::kInvalidEnumValue, Reader(root).ReadObjects(reg, refs, p));

	reg.Register<Color>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "g");
	root[str::kObjects][0][str::kObjectFields]["color"] = "green";
	EXPECT_EQ(ErrorCode::kInvalidEnumValue, Reader(root).ReadObjects(reg, refs, p));

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

	fields["color"] = 5;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	fields["color"] = "#0102ff";
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));

	auto& color = static_cast<U&>(*p).color;
	EXPECT_EQ(1, color.r);
	EXPECT_EQ(2, color.g);
	EXPECT_EQ(255, color.b);
}

TEST(ReaderTest, Variant) {
	Json::Value root;
	RefContainer refs;
	ReferableBase* p = nullptr;
	Registry reg(noasserts);

	reg.Register<V>();

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "v");
	auto& fields = root[str::kObjects][0][str::kObjectFields];
	fields["v"] = 3;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	fields["v"] = Json::objectValue;
	EXPECT_EQ(ErrorCode::kMissingObjectField, Reader(root).ReadObjects(reg, refs, p));

	fields["v"][str::kVariantType] = Json::arrayValue;
	fields["v"][str::kVariantValue] = 22;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	fields["v"][str::kVariantType] = "_i32_";
	EXPECT_EQ(ErrorCode::kUnregisteredType, Reader(root).ReadObjects(reg, refs, p));

	reg.RegisterAll<V>();
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));

	Ref<V> ref;
	EXPECT_TRUE(ref.Resolve(0, p));
	EXPECT_TRUE(bool(ref));

	auto& v = ref->v;
	EXPECT_TRUE(v.Is<int32_t>());
	EXPECT_EQ(22, v.Get<int32_t>());

	fields["v"]["something"] = 1;
	EXPECT_EQ(ErrorCode::kUnexpectedObjectField, Reader(root).ReadObjects(reg, refs, p));
}

TEST(ReaderTest, Versioned) {
	auto ReadAs = [](int version, Json::Value& root, RefContainer& refs, Versioned*& v) -> ErrorCode {
		SetDocVersion(root, version);
		Registry reg(version);
		reg.RegisterAll<Versioned>();
		ReferableBase* p = nullptr;
		auto ec = Reader(root).ReadObjects(reg, refs, p);
		if (p) {
			assert(p->GetTypeId() == serial::StaticTypeId<Versioned>::Get());
			v = static_cast<Versioned*>(p);
		}
		return ec;
	};

	Json::Value root;
	Json::Value dummy;
	RefContainer refs;
	Versioned* v = nullptr;

	root = MakeHeader();
	auto& vs = AddObject(root, MakeObject(0, "versioned"))[str::kObjectFields];

	vs["c"] = "red";
	vs["v"] = MakeVariant("_i32_");
	vs["v"][str::kVariantValue] = 13;
	vs["w"] = MakeVariant("_i32_");
	vs["w"][str::kVariantValue] = 37;
	vs["r"] = "ref_0";
	vs["i3"] = 3;

	// version-0
	EXPECT_EQ(ErrorCode::kNone, ReadAs(0, root, refs, v));
	EXPECT_EQ(Color::kRed, v->color.value);
	EXPECT_EQ(-1, v->i1);
	EXPECT_EQ(-1, v->i2);
	EXPECT_EQ(3, v->i3);
	EXPECT_TRUE(v->v.Is<int>());
	EXPECT_EQ(13, v->v.Get<int>());
	EXPECT_TRUE(v->w.Is<int>());
	EXPECT_EQ(37, v->w.Get<int>());
	EXPECT_TRUE(v->ref.Is<Versioned>());
	EXPECT_EQ(v, v->ref.Get());

	vs["c"] = "orange";
	EXPECT_EQ(ErrorCode::kInvalidEnumValue, ReadAs(0, root, refs, v));

	vs["c"] = "red";
	vs["v"][str::kVariantType] = "_string_";
	vs["v"][str::kVariantValue] = "hello";
	EXPECT_EQ(ErrorCode::kInvalidVariantType, ReadAs(0, root, refs, v));

	// version-1
	EXPECT_EQ(ErrorCode::kMissingObjectField, ReadAs(1, root, refs, v));

	vs["i1"] = 1;
	vs["c"] = "orange";
	EXPECT_EQ(ErrorCode::kNone, ReadAs(1, root, refs, v));
	EXPECT_EQ(Color::kOrange, v->color.value);
	EXPECT_EQ(1, v->i1);
	EXPECT_EQ(-1, v->i2);
	EXPECT_EQ(3, v->i3);
	EXPECT_TRUE(v->v.Is<std::string>());
	EXPECT_EQ(std::string{"hello"}, v->v.Get<std::string>());

	vs["v"][str::kVariantType] = "_f32_";
	vs["v"][str::kVariantValue] = 1.5;
	EXPECT_EQ(ErrorCode::kNone, ReadAs(1, root, refs, v));
	EXPECT_TRUE(v->v.Is<float>());
	EXPECT_EQ(1.5f, v->v.Get<float>());

	vs["i2"] = 2;
	EXPECT_EQ(ErrorCode::kUnexpectedObjectField, ReadAs(1, root, refs, v));

	// version-2 (float is not in variant)
	vs["c"] = "red";
	EXPECT_EQ(ErrorCode::kInvalidVariantType, ReadAs(2, root, refs, v));

	vs["v"][str::kVariantType] = "_i32_";
	vs["v"][str::kVariantValue] = 22;
	vs["c"] = "orange";
	EXPECT_EQ(ErrorCode::kInvalidEnumValue, ReadAs(2, root, refs, v));

	vs["c"] = "red";
	EXPECT_EQ(ErrorCode::kNone, ReadAs(2, root, refs, v));
	EXPECT_EQ(Color::kRed, v->color.value);
	EXPECT_EQ(1, v->i1);
	EXPECT_EQ(2, v->i2);
	EXPECT_EQ(3, v->i3);
	EXPECT_EQ(v, v->ref.Get());
	EXPECT_TRUE(v->v.Is<int>());
	EXPECT_EQ(22, v->v.Get<int>());

	// version-3
	EXPECT_EQ(ErrorCode::kUnexpectedObjectField, ReadAs(3, root, refs, v));

	vs.removeMember("i2");
	vs.removeMember("i3");
	EXPECT_EQ(ErrorCode::kNone, ReadAs(3, root, refs, v));
	EXPECT_EQ(1, v->i1);
	EXPECT_EQ(-1, v->i2);
	EXPECT_EQ(-1, v->i3);

	// -- with referables --

	vs.removeMember("i1");
	vs["i3"] = 3;
	EXPECT_EQ(ErrorCode::kNone, ReadAs(0, root, refs, v));

	// version-0
	auto& ax = AddObject(root, MakeObject(1, "a"))[str::kObjectFields];
	ax["value"] = 12;
	EXPECT_EQ(ErrorCode::kUnregisteredType, ReadAs(0, root, refs, v));

	// version-1
	vs["i1"] = 1;
	EXPECT_EQ(ErrorCode::kNone, ReadAs(1, root, refs, v));

	vs["r"] = "ref_1";
	EXPECT_EQ(ErrorCode::kNone, ReadAs(1, root, refs, v));
	EXPECT_TRUE(v->ref.Is<A>());
	EXPECT_EQ(12, v->ref.As<A>().value);

	auto& bx = AddObject(root, MakeObject(2, "b"))[str::kObjectFields];
	bx["name"] = "hello";
	vs["r"] = "ref_2";
	EXPECT_EQ(ErrorCode::kNone, ReadAs(1, root, refs, v));
	EXPECT_TRUE(v->ref.Is<B>());
	EXPECT_EQ(std::string{"hello"}, v->ref.As<B>().name);

	// version-2
	vs["i2"] = 2;
	EXPECT_EQ(ErrorCode::kUnregisteredType, ReadAs(2, root, refs, v));
	root[str::kObjects].removeIndex(2, &dummy);
	vs["r"] = "ref_1";

	EXPECT_EQ(ErrorCode::kNone, ReadAs(2, root, refs, v));
}
