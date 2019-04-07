#include <iostream>
#include <unordered_set>
#include "serial/Serial.h"
#include "serial/Blueprint.h"
#include "serial/Traverse.h"


using Version1 = serial::Version<1>;
using Version2 = serial::Version<2>;
using Version3 = serial::Version<3>;


struct Winding : serial::Enum {
	enum class Value {
		kClockwise,
		kCounterClockwise,
	} value = {};

	static constexpr auto kTypeName = "winding";

	template<typename Visitor>
	static void AcceptVisitor(Visitor& v) {
		v.VisitEnumValue(Value::kClockwise, "cw");
		v.VisitEnumValue(Value::kCounterClockwise, "ccw", Version2());
	}
};

struct Point {
	int x = 0;
	int y = 0;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.x, "x");
		v.VisitField(self.y, "y");
	}
};

struct Circle : serial::Referable<Circle> {
	int radius = 0;
	Point center;
	Winding winding = {};

	static constexpr auto kTypeName = "circle";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.radius, "radius");
		v.VisitField(self.center, "center", Version1(), Version3());
		v.VisitField(self.winding, "winding");
	}
};

struct A {
	int x = 0;

	static constexpr auto kTypeName = "a";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.x, "x");
	}
};

struct B {
	std::string s;

	static constexpr auto kTypeName = "b";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.s, "s");
	}
};

struct U : serial::UserPrimitive {
	static constexpr auto kTypeName = "u";
	bool ToString(std::string&) const { return true; }
	bool FromString(const std::string&) { return true; }
};

struct Other : serial::Referable<Other> {
	serial::Variant<A, B(Version2)> w;
	U user;

	static constexpr auto kTypeName = "other";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.w, "w");
		v.VisitField(self.user, "user");
	}
};

struct Group : serial::Referable<Group> {
	serial::Array<serial::Ref<Circle, Group, Other(Version3)>> elements;
	serial::Optional<std::string> name;

	static constexpr auto kTypeName = "group";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.elements, "elements");
		v.VisitField(self.name, "name", Version1());
	}
};


// Dump

void Dump(const Json::Value& root) {
	Json::StreamWriterBuilder builder;
	builder.settings_["indentation"] = "  ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &std::cout);
	std::cout << std::endl;
}


void Example() {
	// Model
	Circle c1;
	c1.radius = 121;
	c1.winding.value = Winding::Value::kCounterClockwise;
	c1.center.x = 15;
	c1.center.y = -32;

	Group g1;
	g1.elements.push_back(&c1);
	g1.name = "inner";

	Group g2;
	g2.elements.push_back(&g1);

	Other o1;
	Group g3;
	// g3.elements.push_back(&o1);
	g3.elements.push_back(&c1);
	g3.name = "three";

	B b;
	b.s = "hello";
	o1.w = std::move(b);

	// Serialize
	Json::Value json_value;
	serial::Header header{"example", 3};
	auto ec = serial::Serialize(g3, header, json_value);

	if (ec != serial::ErrorCode::kNone) {
		std::cerr << "error: " << ToString(ec) << std::endl;
		return;
	}

	auto bp1 = serial::Blueprint::FromType<Group>(1);
	auto bp2 = serial::Blueprint::FromType<Group>(2);
	auto bp3 = serial::Blueprint::FromType<Group>(3);
	// std::cerr << bp1 << std::endl;
	// std::cerr << bp2 << std::endl;
	// std::cerr << bp3 << std::endl;

	// std::cerr << Diff(bp1, bp2) << std::endl;
	// std::cerr << Diff(bp2, bp3) << std::endl;

	auto x = R"(
		a :: object
		a.x $ _i32_
		b :: object
		b.s $ _string_
		circle :: referable
		circle.radius $ _i32_
		circle.winding enum winding
		group :: referable
		group.elements[] ref circle
		group.elements[] ref group
		group.elements[] ref other
		group.name? $ _string_
		other :: referable
		other.w variant a
		other.w variant b
		winding :: enum
		winding option ccw
		winding option cw
	)";

	// std::cerr << Diff(serial::Blueprint::FromString(x), bp2) << std::endl;

	return;
	// Dump(json_value);

	// json_value["version"] = 1;

	// Deserialize
	serial::RefContainer refs;
	Group* group = nullptr;
	auto ec2 = serial::DeserializeObjects(json_value, refs, group);
	if (ec2 != serial::ErrorCode::kNone) {
		std::cerr << "error: " << ToString(ec2) << std::endl;
		return;
	}
}

template<typename T>
void func() {
	std::cerr << __PRETTY_FUNCTION__ << std::endl;
}

struct Vis {
	template<typename T> void VisitType() const {
		std::cerr << __PRETTY_FUNCTION__ << std::endl;
	}
};

int main() {
	// Example();
	Vis vv;
	// serial::VisitAllTypes<Group>(vv, 3);
	serial::VisitAllTypes<U>(vv);
}
