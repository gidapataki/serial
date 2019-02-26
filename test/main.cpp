#include <iostream>
#include <vector>
#include "serial/Serial.h"

using namespace serial;


void Dump(const Json::Value& root) {
	Json::StreamWriterBuilder builder;
	builder.settings_["indentation"] = "  ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &std::cout);
	std::cout << std::endl;
}

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


struct Circle : Referable<Circle> {
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


struct Segment : Referable<Segment> {
	Point start;
	Point end;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.start, "start");
		v.VisitField(self.end, "end");
	}
};

struct PolyLine : Referable<PolyLine> {
	Array<Point> points;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.points, "points");
	}
};


struct Group : Referable<Group> {
	Array<BasicRef> elements;
	std::string name;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.elements, "elements");
		v.VisitField(self.name, "name");
	}
};


struct Link : Referable<Link> {
	TypedRef<Circle> circ;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.circ, "circ");
	}
};


void TestSerialize() {
	Registry reg;
	reg.Register<Group>("group");
	reg.Register<Circle>("circle");
	reg.Register<Segment>("segment");

	reg.Register<PolyLine>("polyline");
	reg.Register<Link>("link");

	reg.RegisterEnum<Winding>({
		{Winding::kClockwise, "cw"},
		{Winding::kCounterClockwise, "ccw"},
	});

	Circle c1, c2, c3;
	Segment s1, s2;
	PolyLine p1;
	Link k1;

	c1.radius = 1;
	c2.radius = 2;
	c3.radius = 3;
	c2.winding = Winding::kCounterClockwise;
	s1.start.x = 1;
	s2.start.y = 2;
	k1.circ = &c1;

	p1.points.push_back({});
	p1.points.push_back({});
	p1.points[0].x = 1;
	p1.points[1].x = 2;

	Group g1;
	Group g2;

	g1.name = "g1";
	g2.name = "g2";

	g1.elements.push_back(&s1);
	g1.elements.push_back(&k1);
	g1.elements.push_back(&s1);
	g1.elements.push_back(&c2);
	g1.elements.push_back(&c3);
	g1.elements.push_back(&g2);
	g2.elements.push_back(&s2);
	g2.elements.push_back(&p1);

	Header h;
	h.doctype = "sample";
	h.version = 1;

	Json::Value root;
	ErrorCode ec;
	ec = Serialize(&g1, h, reg, root);
#if 0
	if (ec == ErrorCode::kNone) {
		Dump(root);
	} else {
		std::cerr << "error: " << int(ec) << std::endl;
	}
#else
	RefContainer refs;
	Group* gx = nullptr;

	// Dump(root);
	// std::cerr << root["objects"][1]["type"].asString() << std::endl;
	// root["objects"][1]["fields"]["circ"] = 1;

	ec = DeserializeObjects(root, reg, refs, gx);
	if (!gx) {
		std::cout << "error: " << int(ec) << std::endl;
		return;
	}

	Json::Value root2;
	ec = Serialize(gx, h, reg, root2);
	Dump(root2);
#endif
}


int main() {
	TestSerialize();
	// Segment s;
	// Circle c;
	// PolyLine p;

	// TypedRef<Circle, Segment> ref1, ref2;

	// ref1 = nullptr;
	// ref2 = &c;
	// ref1 = ref2;
	// ref1.Get();
	// ref1.Get<Segment>();
	// std::cout << ref1.Set(nullptr) << std::endl;
}
