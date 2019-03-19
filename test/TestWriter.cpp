#include "gtest/gtest.h"
#include "serial/Ref.h"
#include "serial/Referable.h"
#include "serial/Writer.h"
#include "RgbColor.h"
#include <limits>


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
struct Opt;

using AnyRef = Ref<A, B, C, Leaf, Opt>;

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
	Array<AnyRef> refs;

	static constexpr auto kTypeName = "a";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
		v.VisitField(self.name, "name");
		v.VisitField(self.refs, "refs");
	}
};

struct B : Referable<B> {
	Data data;
	Ref<Leaf> leaf;

	static constexpr auto kTypeName = "b";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.data, "data");
		v.VisitField(self.leaf, "leaf");
	}
};

struct Leaf : Referable<Leaf> {
	static constexpr auto kTypeName = "leaf";

	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct Color : Enum {
	enum Value : int {
		kRed,
		kGreen,
		kBlue,
	} value = {};

	Color() = default;
	Color(Value v) : value(v) {}

	static constexpr auto kTypeName = "color";

	template<typename V>
	static void AcceptVisitor(V& v) {
		v.VisitValue(kRed, "red");
		v.VisitValue(kBlue, "blue");
		// Note: green is not registered
	}
};

struct C : Referable<C> {
	Color color = Color::kRed;

	static constexpr auto kTypeName = "c";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
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
	Optional<Data> d;
	Optional<Array<int>> a;
	Optional<AnyRef> ref;

	static constexpr auto kTypeName = "opt";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.i, "i");
		v.VisitField(self.d, "d");
		v.VisitField(self.a, "a");
		v.VisitField(self.ref, "ref");
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
	EXPECT_TRUE(root[str::kRootId].isString());

	EXPECT_EQ(h.doctype, root[str::kDocType].asString());
	EXPECT_EQ(h.version, root[str::kDocVersion].asInt());

}

void CheckObject(const Json::Value& obj) {
	EXPECT_TRUE(obj.isObject());
	EXPECT_TRUE(obj.isMember(str::kObjectId));
	EXPECT_TRUE(obj.isMember(str::kObjectFields));
	EXPECT_TRUE(obj.isMember(str::kObjectType));
	EXPECT_EQ(3, obj.size());

	EXPECT_TRUE(obj[str::kObjectId].isString());
	EXPECT_TRUE(obj[str::kObjectFields].isObject());
	EXPECT_TRUE(obj[str::kObjectType].isString());

}

void CheckObject(const Json::Value& obj, const std::string& type) {
	CheckObject(obj);
	EXPECT_EQ(type, obj[str::kObjectType].asString());
}

const Json::Value& FirstObjectField(const Json::Value& root, const char* name) {
	return root[str::kObjects][0][str::kObjectFields][name];
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

	EXPECT_TRUE(reg.Register<Leaf>());
	EXPECT_EQ(ErrorCode::kNone, w.Write(h, &leaf, root));

	CheckHeader(root, h);

	auto root_id = root[str::kRootId].asString();
	auto& objs = root[str::kObjects];

	EXPECT_EQ(1, objs.size());
	auto& leaf_obj = objs[0];

	CheckObject(leaf_obj, "leaf");
	EXPECT_EQ(0, leaf_obj[str::kObjectFields].size());
	EXPECT_EQ(root_id, leaf_obj[str::kObjectId].asString());
}

TEST(WriterTest, ObjectTree) {
	Registry reg(noasserts);
	Writer w(reg, noasserts);
	Header h;
	Json::Value root;
	A a1, a2;
	B b;
	Leaf leaf;

	EXPECT_TRUE(reg.Register<A>());
	EXPECT_TRUE(reg.Register<B>());
	EXPECT_TRUE(reg.Register<Leaf>());

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

	auto root_id = root[str::kRootId].asString();
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

	auto a1_id = (*a1_obj)[str::kObjectId].asString();
	auto a2_id = (*a2_obj)[str::kObjectId].asString();
	auto leaf_id = (*leaf_obj)[str::kObjectId].asString();
	auto b_id = (*b_obj)[str::kObjectId].asString();

	auto& a1_refs = (*a1_obj)[str::kObjectFields]["refs"];
	auto& a2_refs = (*a2_obj)[str::kObjectFields]["refs"];
	auto& b_leaf = (*b_obj)[str::kObjectFields]["leaf"];

	EXPECT_EQ(3, a1_refs.size());
	EXPECT_EQ(3, a2_refs.size());

	EXPECT_TRUE(a1_refs[0].isString());
	EXPECT_TRUE(a1_refs[1].isString());
	EXPECT_TRUE(a1_refs[2].isString());
	EXPECT_TRUE(a2_refs[0].isString());
	EXPECT_TRUE(b_leaf.isString());

	EXPECT_EQ(a2_id, a1_refs[0].asString());
	EXPECT_EQ(a2_id, a1_refs[1].asString());
	EXPECT_EQ(leaf_id, a1_refs[2].asString());
	EXPECT_EQ(leaf_id, a2_refs[0].asString());
	EXPECT_EQ(a2_id, a2_refs[1].asString());
	EXPECT_EQ(b_id, a2_refs[2].asString());
	EXPECT_EQ(leaf_id, b_leaf.asString());
}

TEST(WriterTest, EnumValue) {
	Registry reg(noasserts);
	Writer w(reg, noasserts);
	Header h;
	Json::Value root;

	C c;

	reg.Register<C>();
	reg.Register<Color>();
	c.color = Color::kBlue;

	auto ec = w.Write(h, &c, root);
	EXPECT_EQ(ErrorCode::kNone, ec);

	CheckHeader(root, h);
	EXPECT_EQ(1, root[str::kObjects].size());

	auto& c_obj = root[str::kObjects][0];
	CheckObject(c_obj, "c");
	EXPECT_EQ(root[str::kRootId].asString(), c_obj[str::kObjectId].asString());

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
	c.color.value = Color::kGreen;

	reg.Register<C>();
	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Writer(reg, noasserts).Write(h, &c, root));

	reg.Register<Color>();
	EXPECT_EQ(Color::kGreen, c.color.value);
	EXPECT_EQ(nullptr, reg.EnumToString(Color{Color::kGreen}));

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

	reg.Register<A>();
	EXPECT_EQ(ErrorCode::kUnregisteredType, Writer(reg, noasserts).Write(h, &a, root));

	reg.Register<Leaf>();
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &a, root));
}

TEST(WriterTest, NullReferences) {
	Registry reg(noasserts);
	Header h;
	Json::Value root;
	A a;
	B b;
	Leaf leaf;

	reg.Register<A>();
	reg.Register<B>();
	reg.Register<Leaf>();

	a.refs.push_back(nullptr);
	EXPECT_EQ(ErrorCode::kNullReference, Writer(reg, noasserts).Write(h, &a, root));

	a.refs.clear();
	a.refs.push_back(&b);
	EXPECT_EQ(ErrorCode::kNullReference, Writer(reg, noasserts).Write(h, &a, root));

	b.leaf = &leaf;
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &a, root));
}

TEST(WriterTest, AllPrimitives) {
	Registry reg(noasserts);
	Header h;
	Json::Value root;
	All all;

	reg.Register<All>();

	all.b = true;
	all.i32 = 15;
	all.i64 = 137438953473ll;
	all.u32 = 4294967286u;
	all.u64 = 34359738358ull;
	all.f = 2.254e12f;
	all.d = 1.637e215;
	all.s = "yolo";

	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &all, root));

	auto& fields = root[str::kObjects][0][str::kObjectFields];
	EXPECT_TRUE(fields["b"].isBool());
	EXPECT_TRUE(fields["i32"].isInt());
	EXPECT_TRUE(fields["i64"].isInt64());
	EXPECT_TRUE(fields["u32"].isUInt());
	EXPECT_TRUE(fields["u64"].isUInt64());
	EXPECT_TRUE(fields["f"].isDouble());
	EXPECT_TRUE(fields["d"].isDouble());
	EXPECT_TRUE(fields["s"].isString());

	EXPECT_EQ(true, fields["b"].asBool());
	EXPECT_EQ(15, fields["i32"].asInt());
	EXPECT_EQ(137438953473ll, fields["i64"].asInt64());
	EXPECT_EQ(4294967286u, fields["u32"].asUInt());
	EXPECT_EQ(34359738358ull, fields["u64"].asUInt64());
	EXPECT_EQ(2.254e12f, static_cast<float>(fields["f"].asDouble()));
	EXPECT_EQ(1.637e215, fields["d"].asDouble());
	EXPECT_EQ(std::string{"yolo"}, fields["s"].asString());
}

TEST(WriterTest, InfAndNaN) {
	Registry reg(noasserts);
	Header h;
	Json::Value root;
	Floats fs;

	reg.Register<Floats>();

	fs.f = std::numeric_limits<float>::max();
	fs.d = std::numeric_limits<double>::max();
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &fs, root));
	EXPECT_TRUE(FirstObjectField(root, "f").isDouble());
	EXPECT_TRUE(FirstObjectField(root, "d").isDouble());

	fs.f = std::numeric_limits<float>::infinity();
	fs.d = std::numeric_limits<double>::infinity();
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &fs, root));
	EXPECT_TRUE(FirstObjectField(root, "f").isString());
	EXPECT_TRUE(FirstObjectField(root, "d").isString());
	EXPECT_EQ(std::string{"inf"}, FirstObjectField(root, "f").asString());
	EXPECT_EQ(std::string{"inf"}, FirstObjectField(root, "d").asString());

	fs.f = -std::numeric_limits<float>::infinity();
	fs.d = -std::numeric_limits<double>::infinity();
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &fs, root));
	EXPECT_TRUE(FirstObjectField(root, "f").isString());
	EXPECT_TRUE(FirstObjectField(root, "d").isString());
	EXPECT_EQ(std::string{"-inf"}, FirstObjectField(root, "f").asString());
	EXPECT_EQ(std::string{"-inf"}, FirstObjectField(root, "d").asString());

	fs.f = std::numeric_limits<float>::quiet_NaN();
	fs.d = std::numeric_limits<double>::quiet_NaN();
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &fs, root));
	EXPECT_TRUE(FirstObjectField(root, "f").isString());
	EXPECT_TRUE(FirstObjectField(root, "d").isString());
	EXPECT_EQ(std::string{"nan"}, FirstObjectField(root, "f").asString());
	EXPECT_EQ(std::string{"nan"}, FirstObjectField(root, "d").asString());
}

TEST(WriterTest, Optional) {
	Registry reg(noasserts);
	Header h;
	Json::Value root;
	Opt opt;

	reg.Register<Opt>();

	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &opt, root));
	EXPECT_TRUE(FirstObjectField(root, "i").isNull());
	EXPECT_TRUE(FirstObjectField(root, "d").isNull());
	EXPECT_TRUE(FirstObjectField(root, "a").isNull());
	EXPECT_TRUE(FirstObjectField(root, "ref").isNull());

	opt.i = 15;
	opt.d = Data{};
	opt.a = Array<int>({1, 2});
	opt.ref = &opt;
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &opt, root));
	EXPECT_TRUE(FirstObjectField(root, "i").isInt());
	EXPECT_TRUE(FirstObjectField(root, "d").isObject());
	EXPECT_TRUE(FirstObjectField(root, "a").isArray());
	EXPECT_TRUE(FirstObjectField(root, "ref").isString());
	EXPECT_EQ(2, FirstObjectField(root, "a").size());
}

TEST(WriterTest, UserType) {
	Registry reg(noasserts);
	Header h;
	Json::Value root;
	U u;

	reg.Register<U>();
	u.color.r = 255;
	u.color.g = 128;
	u.color.b = 0;
	EXPECT_EQ(ErrorCode::kNone, Writer(reg, noasserts).Write(h, &u, root));
	EXPECT_TRUE(FirstObjectField(root, "color").isString());
	EXPECT_EQ(std::string{"#ff8000"}, FirstObjectField(root, "color").asString());

	u.color.invalid = true;
	EXPECT_EQ(ErrorCode::kUnexpectedValue, Writer(reg, noasserts).Write(h, &u, root));
}
