#include "serial/Serial.h"
#include <iostream>


enum class Winding {
	kClockwise,
	kCounterClockwise,
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

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.radius, "radius");
		v.VisitField(self.center, "center");
		v.VisitField(self.winding, "winding");
	}
};

struct Group : serial::Referable<Group> {
	serial::Array<serial::TypedRef<Circle, Group>> elements;
	serial::Optional<std::string> name;

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
	c1.winding = Winding::kCounterClockwise;
	c1.center.x = 15;
	c1.center.y = -32;

	Group g1;
	g1.elements.push_back(&c1);
	g1.name = "inner";

	Group g2;
	g2.elements.push_back(&g1);

	// Setup registry & co.
	serial::Registry reg;
	serial::Header header{"example", 1};

	reg.Register<Circle>("circle");
	reg.Register<Group>("group");

	reg.RegisterEnum<Winding>({
		{Winding::kClockwise, "cw"},
		{Winding::kCounterClockwise, "ccw"},
	});


	// Serialize

	Json::Value json_value;
	serial::ErrorCode ec = serial::Serialize(&g2, header, reg, json_value);

	// Deserialize

	serial::RefContainer refs;
	Group* group = nullptr;
	auto ec2 = serial::DeserializeObjects(json_value, reg, refs, group);

	Dump(json_value);
}

int main() {
	Example();
}
