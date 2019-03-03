#include "gtest/gtest.h"
#include "serial/BasicRef.h"
#include "serial/TypedRef.h"
#include "serial/Referable.h"
#include "serial/Writer.h"

void Dump(const Json::Value& root) {
	Json::StreamWriterBuilder builder;
	builder.settings_["indentation"] = "  ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &std::cout);
	std::cout << std::endl;
}

using namespace serial;

namespace {



struct A;
struct B;
struct C;
struct Leaf;

struct Data {
	int x = 0;
	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.x, "x");
	}
};

struct A : Referable<A> {
	int value = 0;
	std::string name;
	Array<BasicRef> refs;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
		v.VisitField(self.name, "name");
		v.VisitField(self.refs, "refs");
	}
};

struct B : Referable<B> {
	Data data;
	TypedRef<Leaf> leaf;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.data, "data");
		v.VisitField(self.leaf, "leaf");
	}
};

struct Leaf : Referable<Leaf> {
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

enum class Color {
	kRed,
	kGreen,
	kBlue,
};

struct C : Referable<C> {
	Color color = Color::kRed;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.color, "color");
	}
};


void CheckHeader(const Json::Value& root, const Header& h) {
	EXPECT_TRUE(root.isObject());
	EXPECT_TRUE(root.isMember(str::kDocType));
	EXPECT_TRUE(root.isMember(str::kDocVersion));
	EXPECT_TRUE(root.isMember(str::kObjects));
	EXPECT_TRUE(root.isMember(str::kRootId));
	EXPECT_EQ(4, root.size());

	EXPECT_TRUE(root[str::kDocType].isString());
	EXPECT_TRUE(root[str::kDocVersion].isInt());
	EXPECT_TRUE(root[str::kObjects].isArray());
	EXPECT_TRUE(root[str::kRootId].isInt());

	EXPECT_EQ(h.doctype, root[str::kDocType].asString());
	EXPECT_EQ(h.version, root[str::kDocVersion].asInt());

}

void CheckObject(const Json::Value& obj) {
	EXPECT_TRUE(obj.isObject());
	EXPECT_TRUE(obj.isMember(str::kObjectId));
	EXPECT_TRUE(obj.isMember(str::kObjectFields));
	EXPECT_TRUE(obj.isMember(str::kObjectType));
	EXPECT_EQ(3, obj.size());

	EXPECT_TRUE(obj[str::kObjectId].isInt());
	EXPECT_TRUE(obj[str::kObjectFields].isObject());
	EXPECT_TRUE(obj[str::kObjectType].isString());

}

void CheckObject(const Json::Value& obj, const std::string& type) {
	CheckObject(obj);
	EXPECT_EQ(type, obj[str::kObjectType].asString());
}

} // namespace


TEST(WriterTest, SingleObject) {
	Registry reg(noasserts);
	Writer w(reg, noasserts);
	Header h;
	Json::Value root;
	Leaf leaf;

	h.doctype = "test";
	h.version = 3412;

	EXPECT_TRUE(reg.Register<Leaf>("leaf"));
	EXPECT_EQ(ErrorCode::kNone, w.Write(h, &leaf, root));

	CheckHeader(root, h);

	auto root_id = root[str::kRootId].asInt();
	auto& objs = root[str::kObjects];

	EXPECT_EQ(1, objs.size());
	auto& leaf_obj = objs[0];

	CheckObject(leaf_obj, "leaf");
	EXPECT_EQ(0, leaf_obj[str::kObjectFields].size());
	EXPECT_EQ(root_id, leaf_obj[str::kObjectId].asInt());
}

TEST(WriterTest, ObjectTree) {
	Registry reg(noasserts);
	Writer w(reg, noasserts);
	Header h;
	Json::Value root;
	A a1, a2;
	B b;
	Leaf leaf;

	EXPECT_TRUE(reg.Register<A>("a"));
	EXPECT_TRUE(reg.Register<B>("b"));
	EXPECT_TRUE(reg.Register<Leaf>("leaf"));

	a1.name = "a1";
	a1.value = 12;
	a2.name = "a2";
	a2.value = 23;
	b.data.x = 47;
	b.leaf = &leaf;

	a1.refs.push_back(&a2);
	a1.refs.push_back(&a2);
	a1.refs.push_back(&leaf);
	a2.refs.push_back(&leaf);
	a2.refs.push_back(&a2);
	a2.refs.push_back(&b);

	EXPECT_EQ(ErrorCode::kNone, w.Write(h, &a1, root));

	CheckHeader(root, h);

	auto root_id = root[str::kRootId].asInt();
	auto& objs = root[str::kObjects];

	EXPECT_EQ(4, objs.size());

	Json::Value* a1_obj = nullptr;
	Json::Value* a2_obj = nullptr;
	Json::Value* b_obj = nullptr;
	Json::Value* leaf_obj = nullptr;

	for (int i = 0; i < objs.size(); ++i) {
		auto& current = objs[i];
		CheckObject(objs[i]);
		auto type = current[str::kObjectType].asString();
		if (type == "leaf") {
			EXPECT_EQ(nullptr, leaf_obj);
			EXPECT_EQ(0, current[str::kObjectFields].size());

			leaf_obj = &objs[i];
		} else if (type == "b") {
			EXPECT_EQ(2, current[str::kObjectFields].size());
			EXPECT_TRUE(current[str::kObjectFields].isMember("data"));
			EXPECT_TRUE(current[str::kObjectFields].isMember("leaf"));
			EXPECT_EQ(nullptr, b_obj);
			b_obj = &current;
		} else {
			EXPECT_EQ(3, current[str::kObjectFields].size());
			EXPECT_TRUE(current[str::kObjectFields].isMember("name"));
			EXPECT_TRUE(current[str::kObjectFields].isMember("value"));
			EXPECT_TRUE(current[str::kObjectFields].isMember("refs"));

			EXPECT_TRUE(current[str::kObjectFields]["name"].isString());
			EXPECT_EQ(std::string{"a"}, current[str::kObjectType].asString());

			auto name = current[str::kObjectFields]["name"].asString();
			EXPECT_TRUE(name == "a1" || name == "a2");
			if (name == "a1") {
				EXPECT_EQ(nullptr, a1_obj);
				a1_obj = &current;
			} else if (name == "a2") {
				EXPECT_EQ(nullptr, a2_obj);
				a2_obj = &current;
			}
		}
	}

	EXPECT_NE(nullptr, a1_obj);
	EXPECT_NE(nullptr, a2_obj);
	EXPECT_NE(nullptr, leaf_obj);
	EXPECT_NE(nullptr, b_obj);

	EXPECT_EQ(12, (*a1_obj)[str::kObjectFields]["value"].asInt());
	EXPECT_EQ(23, (*a2_obj)[str::kObjectFields]["value"].asInt());

	auto& data_field = (*b_obj)[str::kObjectFields]["data"];
	EXPECT_TRUE(data_field.isObject());
	EXPECT_EQ(1, data_field.size());

	EXPECT_TRUE(data_field.isMember("x"));
	EXPECT_TRUE(data_field["x"].isInt());
	EXPECT_EQ(47, data_field["x"].asInt());

	auto a1_id = (*a1_obj)[str::kObjectId].asInt();
	auto a2_id = (*a2_obj)[str::kObjectId].asInt();
	auto leaf_id = (*leaf_obj)[str::kObjectId].asInt();
	auto b_id = (*b_obj)[str::kObjectId].asInt();

	auto& a1_refs = (*a1_obj)[str::kObjectFields]["refs"];
	auto& a2_refs = (*a2_obj)[str::kObjectFields]["refs"];
	auto& b_leaf = (*b_obj)[str::kObjectFields]["leaf"];

	EXPECT_EQ(3, a1_refs.size());
	EXPECT_EQ(3, a2_refs.size());

	EXPECT_TRUE(a1_refs[0].isInt());
	EXPECT_TRUE(a1_refs[1].isInt());
	EXPECT_TRUE(a1_refs[2].isInt());
	EXPECT_TRUE(a2_refs[0].isInt());
	EXPECT_TRUE(b_leaf.isInt());

	EXPECT_EQ(a2_id, a1_refs[0].asInt());
	EXPECT_EQ(a2_id, a1_refs[1].asInt());
	EXPECT_EQ(leaf_id, a1_refs[2].asInt());
	EXPECT_EQ(leaf_id, a2_refs[0].asInt());
	EXPECT_EQ(a2_id, a2_refs[1].asInt());
	EXPECT_EQ(b_id, a2_refs[2].asInt());
	EXPECT_EQ(leaf_id, b_leaf.asInt());
}

TEST(WriterTest, EnumValue) {
	Registry reg(noasserts);
	Writer w(reg, noasserts);
	Header h;
	Json::Value root;

	C c;

	reg.Register<C>("c");
	reg.RegisterEnum<Color>({{Color::kRed, "red"}, {Color::kBlue, "blue"}});
	c.color = Color::kBlue;

	auto ec = w.Write(h, &c, root);
	EXPECT_EQ(ErrorCode::kNone, ec);

	CheckHeader(root, h);
	EXPECT_EQ(1, root[str::kObjects].size());

	auto& c_obj = root[str::kObjects][0];
	CheckObject(c_obj, "c");
	EXPECT_EQ(root[str::kRootId].asInt(), c_obj[str::kObjectId].asInt());

	auto& c_fields = c_obj[str::kObjectFields];
	EXPECT_EQ(1, c_fields.size());
	EXPECT_TRUE(c_fields.isMember("color"));
	EXPECT_TRUE(c_fields["color"].isString());
	EXPECT_EQ(std::string{"blue"}, c_fields["color"].asString());
}

TEST(WriterTest, UnregisteredEnum) {
	Registry reg(noasserts);
	Header h;
	Json::Value root;

	C c;
	c.color = Color::kGreen;

	reg.Register<C>("c");
	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Writer(reg, noasserts).Write(h, &c, root));

	reg.RegisterEnum<Color>({{Color::kBlue, "blue"}});
	EXPECT_EQ(Color::kGreen, c.color);
	EXPECT_EQ(nullptr, reg.EnumToString(Color::kGreen));

	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Writer(reg, noasserts).Write(h, &c, root));

	c.color = Color::kBlue;
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &c, root));
}

TEST(WriterTest, UnregisteredType) {
	Registry reg(noasserts);
	Writer w(reg, noasserts);
	Header h;
	Json::Value root;
	A a;
	Leaf leaf;

	a.refs.push_back(&leaf);

	EXPECT_EQ(ErrorCode::kUnregisteredType, Writer(reg, noasserts).Write(h, &a, root));

	reg.Register<A>("a");
	EXPECT_EQ(ErrorCode::kUnregisteredType, Writer(reg, noasserts).Write(h, &a, root));

	reg.Register<Leaf>("leaf");
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &a, root));
}

TEST(WriterTest, NullReferences) {
	Registry reg(noasserts);
	Header h;
	Json::Value root;
	A a;
	B b;
	Leaf leaf;

	reg.Register<A>("a");
	reg.Register<B>("b");
	reg.Register<Leaf>("leaf");

	a.refs.push_back(nullptr);
	EXPECT_EQ(ErrorCode::kNullReference, Writer(reg, noasserts).Write(h, &a, root));

	a.refs.clear();
	a.refs.push_back(&b);
	EXPECT_EQ(ErrorCode::kNullReference, Writer(reg, noasserts).Write(h, &a, root));

	b.leaf = &leaf;
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &a, root));
}

