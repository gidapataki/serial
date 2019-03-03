#include "gtest/gtest.h"
#include "serial/BasicRef.h"
#include "serial/TypedRef.h"
#include "serial/Referable.h"
#include "serial/Reader.h"


using namespace serial;

namespace {

Json::Value MakeHeader(int root_id = 0) {
	Json::Value root;
	root = Json::Value(Json::objectValue);
	root[str::kDocType] = "test";
	root[str::kDocVersion] = 1;
	root[str::kRootId] = root_id;
	root[str::kObjects] = Json::Value(Json::arrayValue);
	return root;
}

Json::Value MakeObject(int id, const char* type) {
	Json::Value root;
	root[str::kObjectType] = type;
	root[str::kObjectFields] = Json::objectValue;
	root[str::kObjectId] = id;
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

enum class Color {
	kRed,
	kGreen,
	kBlue,
};


struct Leaf : Referable<Leaf> {
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct A : Referable<A> {
	int value = 0;
	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
	}
};

struct B : Referable<B> {
	std::string name;
	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.name, "name");
	}
};

struct C : Referable<C> {
	BasicRef ref;
	Array<TypedRef<A, Leaf>> elements;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.ref, "ref");
		v.VisitField(self.elements, "elements");
	}
};

struct D : Referable<D> {
	std::string name;
	int value = 0;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.name, "name");
		v.VisitField(self.value, "value");
	}
};

struct E : Referable<E> {
	Array<int> values;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.values, "values");
	}
};

struct F : Referable<F> {
	Point point;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.point, "point");
	}
};

struct G : Referable<G> {
	Color color = {};

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.color, "color");
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
	reg.Register<Leaf>("leaf");

	refs.push_back(nullptr);
	refs.push_back(nullptr);

	root = Json::arrayValue;
	EXPECT_EQ(ErrorCode::kInvalidDocument, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	root[str::kRootId] = "hello";
	EXPECT_EQ(ErrorCode::kInvalidHeader, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	root[str::kObjects] = 23;
	EXPECT_EQ(ErrorCode::kMissingRootObject, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	EXPECT_EQ(ErrorCode::kMissingRootObject, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	root[str::kRootId] = 1;
	root[str::kObjects][0] = MakeObject(15, "leaf");
	EXPECT_EQ(ErrorCode::kMissingRootObject, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	root[str::kRootId] = 1;
	root[str::kObjects][0] = MakeObject(1, "leaf");
	root[str::kObjects][0]["id"] = Json::nullValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectHeader, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_EQ(&l2, p);

	root = MakeHeader();
	root[str::kRootId] = 12;
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
	reg.Register<Leaf>("leaf");
	reg.Register<A>("a");
	reg.Register<B>("b");
	reg.Register<C>("c");
	reg.Register<D>("d");

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
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = 22;
	root[str::kObjects][0][str::kObjectFields]["ref"] = 1;
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kUnresolvableReference, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = 0;
	root[str::kObjects][0][str::kObjectFields]["ref"] = 1;
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kInvalidReferenceType, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = 1;
	root[str::kObjects][0][str::kObjectFields]["ref"] = 22;
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
	reg.Register<E>("e");

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
	reg.Register<F>("f");

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
	reg.Register<Leaf>("leaf");
	reg.Register<C>("c");
	reg.Register<G>("g");

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "g");
	root[str::kObjects][0][str::kObjectFields]["color"] = 5;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "g");
	root[str::kObjects][0][str::kObjectFields]["color"] = "green";
	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Reader(root).ReadObjects(reg, refs, p));

	reg.RegisterEnum<Color>({{Color::kRed, "red"}, {Color::kBlue, "blue"}});

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
	reg.Register<G>("a");
	reg.Register<C>("c");
	reg.Register<Leaf>("leaf");

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["ref"] = "hello";
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["ref"] = 0;
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = "xy";
	EXPECT_EQ(ErrorCode::kInvalidObjectField, Reader(root).ReadObjects(reg, refs, p));

	root = MakeHeader(0);
	root[str::kObjects][0] = MakeObject(0, "c");
	root[str::kObjects][0][str::kObjectFields]["ref"] = 0;
	root[str::kObjects][0][str::kObjectFields]["elements"] = Json::arrayValue;
	root[str::kObjects][0][str::kObjectFields]["elements"][0] = 1;
	root[str::kObjects][1] = MakeObject(1, "leaf");
	EXPECT_EQ(ErrorCode::kNone, Reader(root).ReadObjects(reg, refs, p));
	EXPECT_EQ(2, refs.size());
	EXPECT_TRUE(refs[0].get() == p || refs[1].get() == p);
	EXPECT_EQ(StaticTypeId<C>::Get(), p->GetTypeId());
}
