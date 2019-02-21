#include <cassert>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>


class Reader;
class Writer;

class SerializableBase {
public:
	virtual ~SerializableBase() {}
	virtual void Write(Writer& writer) const = 0;
};

template<typename T>
class Serializable : public SerializableBase {
public:
	virtual void Write(Writer& writer) const override;
};


template<typename T> using Array = std::vector<T>;

using Ref = const SerializableBase*;


struct PrimitiveTag {};
struct ArrayTag {};
struct ObjectTag {};
struct RefTag {};


template<typename T>
struct TypeTag {
	using Type = ObjectTag;
};

template<typename T>
struct TypeTag<Array<T>> {
	using Type = ArrayTag;
};

template<> struct TypeTag<Ref> { using Type = RefTag; };
template<> struct TypeTag<int> { using Type = PrimitiveTag; };

template<typename T>
struct TypeId {
	static void* Get() {
		static TypeId id;
		return reinterpret_cast<void*>(&id);
	}
};

class Registry {
public:
	template<typename T>
	void Register(const char* name) {
		types_[TypeId<T>::Get()] = name;
	}

	template<typename T>
	const char* GetName() const {
		auto id = TypeId<T>::Get();
		auto p = types_.find(id);
		if (p == types_.end()) {
			return nullptr;
		}

		return p->second;
	}

private:
	std::unordered_map<void*, const char*> types_;
};

class Writer {
public:
	Writer(const Registry& reg) : reg_(reg) {}

	template<typename T>
	void VisitField(const T& value, const char* name) {
		Output() << "" << name << " = ";
		VisitValue(value);
	}

	template<typename T>
	void Write(const T& value) {
		auto refid = refids_[&value];
		auto name = reg_.GetName<T>();

		assert(name != nullptr && "Invalid type");

		Output() << name << "(" << std::endl;
		++indent_;
		Output() << "@" << refid << std::endl;
		T::AcceptVisitor(value, *this);
		--indent_;
		Output() << ")" << std::endl;
	}

	void Add(Ref ref) {
		if (refids_.count(ref) == 0) {
			stack_.insert(ref);
			refids_[ref] = next_refid_++;
		}
	}

	void Write() {
		while (!stack_.empty()) {
			auto p = stack_.begin();
			auto ref = *p;
			stack_.erase(p);
			ref->Write(*this);
		}
	}

private:
	std::ostream& Output() {
		return Indent(std::cout);
	}

	std::ostream& Indent(std::ostream& stream) {
		for (int i = 0; i < indent_; ++i) {
			stream << "  ";
		}
		return stream;
	}

	template<typename T>
	void VisitValue(const T& value) {
		typename TypeTag<T>::Type tag;
		VisitValue(value, tag);
	}

	template<typename T>
	void VisitValue(const T& value, RefTag) {
		Add(value);
		std::cout << "@" << refids_[value] << std::endl;
	}

	template<typename T>
	void VisitValue(const T& value, PrimitiveTag) {
		std::cout << value << std::endl;
	}

	template<typename T>
	void VisitValue(const T& value, ArrayTag) {
		std::cout << "[" << std::endl;
		++indent_;
		for (auto& item : value) {
			Output();
			VisitValue(item);
		}
		--indent_;
		Output() << "]" << std::endl;
	}

	template<typename T>
	void VisitValue(const T& value, ObjectTag) {
		std::cout << "{" << std::endl;
		++indent_;
		T::AcceptVisitor(value, *this);
		--indent_;
		Output() << "}" << std::endl;
	}

	int indent_ = 0;
	int next_refid_ = 0;
	std::unordered_map<Ref, int> refids_;
	std::unordered_set<Ref> stack_;
	const Registry& reg_;
};

template<typename T>
void Serializable<T>::Write(Writer& writer) const {
	writer.Write(static_cast<const T&>(*this));
}

void Serialize(const Registry& reg, Ref ref) {
	Writer w(reg);
	w.Add(ref);
	w.Write();
}


// User types //////////////////////////////////////////////////////////////////

struct Point {
	int x = 0;
	int y = 0;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.x, "x");
		v.VisitField(self.y, "y");
	}
};


struct Circle : Serializable<Circle> {
	int radius = 0;
	Point center;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.radius, "radius");
		v.VisitField(self.center, "center");
	}
};


struct Segment : Serializable<Segment> {
	Point start;
	Point end;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.start, "start");
		v.VisitField(self.end, "end");
	}
};


struct Group : Serializable<Group> {
	Array<Ref> elements;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.elements, "elements");
	}
};


int main() {
	Registry reg;
	reg.Register<Group>("group");
	reg.Register<Circle>("circle");
	reg.Register<Segment>("segment");

	Circle c1, c2, c3;
	Segment s1, s2;

	c1.radius = 1;
	c2.radius = 2;
	c3.radius = 3;
	s1.start.x = 1;
	s2.start.y = 2;

	Group g1;
	Group g2;

	g1.elements.push_back(&c1);
	g1.elements.push_back(&s1);
	g1.elements.push_back(&c2);
	g1.elements.push_back(&c3);
	g1.elements.push_back(&g2);
	g2.elements.push_back(&s2);

	Serialize(reg, &g1);

	return 0;
}
