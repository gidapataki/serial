#include <iostream>
#include "serial/Serial.h"

using namespace serial;


void Dump(const Json::Value& root) {
	Json::StreamWriterBuilder builder;
	builder.settings_["indentation"] = "  ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &std::cout);
	std::cout << std::endl;
}


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

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.radius, "radius");
		v.VisitField(self.center, "center");
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
	Array<Ref> elements;
	std::string name;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.elements, "elements");
		v.VisitField(self.name, "name");
	}
};


void TestSerialize() {
	Registry reg;
	reg.Register<Group>("group");
	reg.Register<Circle>("circle");
	reg.Register<Segment>("segment");
	reg.Register<PolyLine>("polyline");

	Circle c1, c2, c3;
	Segment s1, s2;
	PolyLine p1;

	c1.radius = 1;
	c2.radius = 2;
	c3.radius = 3;
	s1.start.x = 1;
	s2.start.y = 2;

	p1.points.push_back({});
	p1.points.push_back({});
	p1.points[0].x = 1;
	p1.points[1].x = 2;

	Group g1;
	Group g2;

	g1.name = "g1";
	g2.name = "g2";

	g1.elements.push_back(&c1);
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
	// Dump(root);

	std::vector<UniqueRef> refs;
	ec = DeserializeObjects(root, reg, refs);

	Json::Value root2;
	ec = Serialize(refs[0].get(), h, reg, root2);
	Dump(root2);
}


int main() {
	TestSerialize();
}
