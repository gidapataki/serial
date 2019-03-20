#include "serial/Serial.h"
#include "serial/Variant.h"
#include <iostream>


struct Winding : serial::Enum {
	enum class Value {
		kClockwise,
		kCounterClockwise,
	} value = {};

	static constexpr auto kTypeName = "winding";

	template<typename Visitor>
	static void AcceptVisitor(Visitor& v) {
		v.VisitValue(Value::kClockwise, "cw");
		v.VisitValue(Value::kCounterClockwise, "ccw");
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
		v.VisitField(self.center, "center");
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

struct Other : serial::Referable<Other> {
	serial::Variant<A, B> w;

	static constexpr auto kTypeName = "other";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.w, "w");
	}
};

struct Group : serial::Referable<Group> {
	serial::Array<serial::Ref<Circle, Group, Other>> elements;
	serial::Optional<std::string> name;

	static constexpr auto kTypeName = "group";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.elements, "elements");
		v.VisitField(self.name, "name");
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
	g3.elements.push_back(&o1);

	B b;
	b.s = "hello";
	o1.w = std::move(b);

	// Serialize
	Json::Value json_value;
	serial::Header header{"example", 1};
	auto ec = serial::Serialize(g3, header, json_value);

	if (ec != serial::ErrorCode::kNone) {
		std::cerr << "error: " << ToString(ec) << std::endl;
		return;
	}
	Dump(json_value);

	// Deserialize
	// serial::RefContainer refs;
	// Group* group = nullptr;
	// auto ec2 = serial::DeserializeObjects(json_value, refs, group);

	// Dump(json_value);
}

int main() {
	Example();
}

