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

	static constexpr auto kReferableName = "circle";

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

	static constexpr auto kReferableName = "segment";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.start, "start");
		v.VisitField(self.end, "end");
	}
};


struct PolyLine : Referable<PolyLine> {
	Array<Point> points;

	static constexpr auto kReferableName = "poly";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.points, "points");
	}
};

struct Link;

struct Group : Referable<Group> {
	Array<TypedRef<PolyLine, Segment, Circle, Group, Link>> elements;
	std::string name;

	static constexpr auto kReferableName = "group";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.elements, "elements");
		v.VisitField(self.name, "name");
	}
};


struct Link : Referable<Link> {
	TypedRef<Circle, Segment> circ;

	static constexpr auto kReferableName = "link";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.circ, "circ");
	}
};


void TestSerialize() {
	Registry reg;
	reg.Register<Group>();
	reg.Register<Circle>();
	reg.Register<Segment>();

	reg.Register<PolyLine>();
	reg.Register<Link>();

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
	// g1.elements.push_back(&c3);
	// g1.elements.push_back(&g2);
	// g2.elements.push_back(&s2);
	// g2.elements.push_back(&p1);

	Header h;
	h.doctype = "sample";
	h.version = 1;

	Json::Value root;
	ErrorCode ec;
	ec = Serialize(&g1, h, reg, root);
		Dump(root);
#if 0
	if (ec == ErrorCode::kNone) {
		Dump(root);
	} else {
		std::cerr << "error: " << int(ec) << std::endl;
	}
#else
	RefContainer refs;
	TypedRef<Group>
		gx = nullptr;

	// Dump(root);
	// std::cerr << root["objects"][1]["type"].asString() << std::endl;
	// root["objects"][1]["fields"]["circ"] = 1;

	ec = DeserializeObjects(root, reg, refs, gx);
	if (!gx) {
		std::cout << "error: " << ToString(ec) << std::endl;
		return;
	}

	Json::Value root2;
	ec = Serialize(gx.Get(), h, reg, root2);
	Dump(root2);
#endif
}


void TestRange() {
	Json::Value root;
	auto& v = root["long"];
	v = 8589934592ull;
	v = 4294966796ull;
	v = 6.23e201;

	std::cout << v << std::endl;
	std::cout << v.isInt() << std::endl;
	std::cout << v.isUInt() << std::endl;
	std::cout << v.isInt64() << std::endl;
	std::cout << v.isDouble() << std::endl;
	std::cout << v.asFloat() << std::endl;
	std::cout << v.asDouble() << std::endl;
}


struct All : Referable<All> {
	bool b = true;
	int i = 15;
	int64_t i64 = 1447632456543;
	unsigned u = -1;
	uint64_t u64 = -1ull;
	float f = 2532445621.214f;
	double d = 21425324456213674343.2153654;
	std::string s = "hi mom";
	Optional<int> o;

	static constexpr auto kReferableName = "all";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.b, "b");
		v.VisitField(self.i, "i");
		v.VisitField(self.i64, "i64");
		v.VisitField(self.u, "u");
		v.VisitField(self.u64, "u64");
		v.VisitField(self.s, "s");
		v.VisitField(self.f, "f");
		v.VisitField(self.d, "d");
		v.VisitField(self.o, "o");
	}
};

void TestPrimitives() {
	Registry reg;
	All all;
	Json::Value root;
	ErrorCode ec;

	reg.Register<All>();

	all.d = std::numeric_limits<double>::lowest();
	all.o = 15;

	ec = Serialize(&all, Header{}, reg, root);
	if (ec != ErrorCode::kNone) {
		return;
	}
	Dump(root);

// return;
	RefContainer refs;
	BasicRef obj;
	ec = DeserializeObjects(root, reg, refs, obj);
	if (ec != ErrorCode::kNone) {
		return;
	}

	ec = Serialize(obj, Header{}, reg, root);
	if (ec != ErrorCode::kNone) {
		return;
	}
	Dump(root);
}


struct Simple : Referable<Simple> {
	Array<Array<Optional<int32_t>>> arr;
	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.arr, "arr");
	}
};

class Vx {
	struct State {
		std::string prefix;
	} state_;

	class StateSentry {
	public:
		StateSentry(Vx* parent)
			: parent_(parent)
			, state_(parent->state_)
		{}

		~StateSentry() {
			parent_->state_ = state_;
		}

	private:
		State state_;
		Vx* parent_ = nullptr;
	};

public:
	template<typename T>
	void Dump(const char* prefix) {
		StateSentry sentry(this);
		state_.prefix = prefix;
		T elem;
		T::AcceptVisitor(elem, *this);
	}

	template<typename T>
	void VisitField(const T& value, const char* name) {
		StateSentry sentry(this);
		state_.prefix += '.';
		state_.prefix += name;
		VisitValue(value);
	}

	template<typename T>
	void VisitValue(const T& value) {
		using Tag = typename TypeTag<T>::Type;
		Tag tag;
		VisitValue(value, tag);
	}

	template<typename T>
	void VisitValue(const T& value, PrimitiveTag) {
		VisitPrimitive(value);
	}

	void DeclareType(const char* name) {
		std::cerr << state_.prefix << " : " << name << std::endl;
	}

	void VisitPrimitive(const bool&) { DeclareType("bool"); }
	void VisitPrimitive(const int32_t&) { DeclareType("i32"); }
	void VisitPrimitive(const int64_t&) { DeclareType("i64"); }
	void VisitPrimitive(const uint32_t&) { DeclareType("u32"); }
	void VisitPrimitive(const uint64_t&) { DeclareType("u64"); }
	void VisitPrimitive(const float&) { DeclareType("float"); }
	void VisitPrimitive(const double&) { DeclareType("double"); }
	void VisitPrimitive(const std::string&) { DeclareType("string"); }

	template<typename T>
	void VisitValue(const Array<T>& value, ArrayTag) {
		StateSentry sentry(this);

		state_.prefix += "[]";
		T elem;
		VisitValue(elem);
	}

	template<typename T>
	void VisitValue(const Optional<T>& value, OptionalTag) {
		StateSentry sentry(this);

		state_.prefix += "?";
		T elem;
		VisitValue(elem);
	}

	template<typename T>
	void VisitValue(const T& value, ObjectTag) {
		StateSentry sentry(this);
		T::AcceptVisitor(value, *this);
	}

	template<typename T>
	void VisitValue(const T& value, EnumTag) {
		DeclareType("enum");
	}

	template<typename T>
	void VisitValue(const T& value, BasicRefTag) {
		DeclareType("ref");
	}

	template<typename T>
	void VisitValue(const T& value, TypedRefTag) {
		DeclareType("ref*");
	}

	template<typename T>
	void VisitValue(const T& value, UserTag) {
		DeclareType("user");
	}
};


void DumpStructure() {
	Vx vx;
	vx.Dump<Circle>("circle");
	vx.Dump<PolyLine>("poly");
	vx.Dump<Simple>("simple");

}

void Stress() {
	Registry reg;
	Json::Value root;
	ErrorCode ec;
	Header h;

	reg.Register<Group>();
	reg.Register<PolyLine>();

	Group g;
	std::vector<PolyLine> ps;
	int N = 100000;
	ps.reserve(N);

	for (int i = 0; i < N; ++i) {
		PolyLine p;
		// p.points.push_back({i, i});
		// p.points.push_back({i + 1, i + 1});
		// p.points.push_back({i + 2, i + 2});
		ps.push_back(std::move(p));
	}

	for (auto& p : ps) {
		g.elements.push_back(&p);
	}

	ec = Serialize(&g, h, reg, root);
}


struct Sol {
	std::size_t len = 0;

	bool FromString(const std::string& str) {
		len = str.size();
		return true;
	}

	bool ToString(std::string& str) {
		std::string s;
		for (auto i = len; len-- > 0;) {
			s += '.';
		}
		str = s;
		return true;
	}

};


struct XEnum {
	enum Value {
		kOne,
		kTwo
	} value;


};


enum class YEnum {
	kOne,
	kTwo
};


template<typename T>
void func() {

}

int main() {
}

