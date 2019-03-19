#include "serial/Serial.h"
// #include "serial/RegistryBuilder.h"
#include <iostream>


struct Winding : serial::Enum {
	enum class Value {
		kClockwise,
		kCounterClockwise,
	} value = {};

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

	static constexpr auto kReferableName = "circle";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.radius, "radius");
		v.VisitField(self.center, "center");
		v.VisitField(self.winding, "winding");
	}
};

struct Group : serial::Referable<Group> {
	serial::Array<serial::Ref<Circle, Group>> elements;
	serial::Optional<std::string> name;

	static constexpr auto kReferableName = "group";

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


	// Setup registry
	serial::Registry reg;

#if 1
	reg.RegisterAll<Group>();
#else
	reg.Register<Circle>();
	reg.Register<Group>();
	reg.RegisterEnum<Winding>();
#endif

	// Serialize
	Json::Value json_value;
	serial::Header header{"example", 1};
	auto ec = serial::Serialize(g2, header, reg, json_value);

	if (ec != serial::ErrorCode::kNone) {
		std::cerr << ToString(ec) << std::endl;
		return;
	}
	Dump(json_value);

	// Deserialize
	// serial::RefContainer refs;
	// Group* group = nullptr;
	// auto ec2 = serial::DeserializeObjects(json_value, reg, refs, group);

	// Dump(json_value);
}

int main() {
	Example();
}

