#include <cassert>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "jsoncpp/json.h"


void Dump(Json::Value& root) {
	Json::StreamWriterBuilder builder;
	builder.settings_["indentation"] = "  ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &std::cout);
	std::cout << std::endl;
}


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
	class StateSentry {
	public:
		StateSentry(Writer* writer)
			: parent_(writer)
			, current_(writer->current_)
		{}

		~StateSentry() {
			parent_->current_ = current_;
		}

	private:
		Writer* parent_;
		Json::Value* current_;
	};

public:
	Writer(const Registry& reg) : reg_(reg) {}

	template<typename T>
	void VisitField(const T& value, const char* name) {
		StateSentry sentry(this);
		Select(name);
		VisitValue(value);
	}

	template<typename T>
	void Write(const T& value) {
		auto refid = refids_[&value];
		auto name = reg_.GetName<T>();

		assert(name != nullptr && "Invalid type");

		StateSentry sentry(this);
		Current()["T"] = name;
		Current()["R"] = Json::Value(refid);
		Select("V");
		T::AcceptVisitor(value, *this);
	}

	void Add(Ref ref) {
		if (refids_.count(ref) == 0) {
			stack_.insert(ref);
			refids_[ref] = next_refid_++;
		}
	}

	void Write() {
		StateSentry sentry(this);
		Select("objects") = Json::Value(Json::arrayValue);

		while (!stack_.empty()) {
			StateSentry sentry2(this);
			SelectNext();
			auto p = stack_.begin();
			auto ref = *p;
			stack_.erase(p);
			ref->Write(*this);
		}

		Dump(root_);
	}

private:
	Json::Value& Select(const char* name) {
		current_ = &Current()[name];
		return Current();
	}

	Json::Value& SelectNext() {
		current_ = &Current().append({});
		return Current();
	}

	Json::Value& Current() {
		return *current_;
	}

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
		auto refid = refids_[value];
		Current() = Json::Value(refid);
	}

	template<typename T>
	void VisitValue(const T& value, PrimitiveTag) {
		Current() = Json::Value(value);
	}

	template<typename T>
	void VisitValue(const T& value, ArrayTag) {
		StateSentry sentry(this);
		Current() = Json::Value(Json::arrayValue);
		++indent_;
		for (auto& item : value) {
			StateSentry sentry2(this);
			SelectNext();
			VisitValue(item);
		}
		--indent_;
	}

	template<typename T>
	void VisitValue(const T& value, ObjectTag) {
		++indent_;
		T::AcceptVisitor(value, *this);
		--indent_;
	}

	int next_refid_ = 0;
	std::unordered_map<Ref, int> refids_;
	std::unordered_set<Ref> stack_;
	const Registry& reg_;

	Json::Value root_;
	Json::Value* current_ = &root_;
	int indent_ = 0;
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


void CheckSerial() {
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
}


// Json

void CheckJson() {
	Json::Value root;
	root["a"];
	Dump(root);
}

int main() {
	// CheckJson();
	CheckSerial();
	return 0;
}
