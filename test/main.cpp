#include <cassert>
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "jsoncpp/json.h"


namespace str {

constexpr const char* kDoctype = "doctype";
constexpr const char* kVersion = "version";
constexpr const char* kObjects = "objects";
constexpr const char* kType = "type";
constexpr const char* kFields = "fields";
constexpr const char* kId = "id";

} // namespace str


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
	virtual void Write(Writer* writer) const = 0;
	virtual bool Read(Reader* reader) = 0;
};

template<typename T>
class Serializable : public SerializableBase {
public:
	virtual void Write(Writer* writer) const override;
	virtual bool Read(Reader* reader) override;
};


template<typename T> using Array = std::vector<T>;

using Ref = const SerializableBase*;
using UniqueRef = std::unique_ptr<SerializableBase>;

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
template<> struct TypeTag<std::string> { using Type = PrimitiveTag; };

template<typename T>
struct TypeId {
	static void* Get() {
		static TypeId id;
		return reinterpret_cast<void*>(&id);
	}
};


class FactoryBase {
public:
	virtual ~FactoryBase() {}
	virtual UniqueRef Create() const = 0;
};


template<typename T>
class Factory : public FactoryBase {
public:
	static_assert(std::is_base_of<SerializableBase, T>::value,
		"Type is not serializable");

	virtual UniqueRef Create() const override {
		return std::make_unique<T>();
	}
};


class Registry {
public:
	template<typename T>
	void Register(const char* name) {
		assert(factories_.find(name) == factories_.end() &&
			"Duplicate type name");
		types_[TypeId<T>::Get()] = name;
		factories_[name] = std::make_unique<Factory<T>>();
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

	UniqueRef Create(const std::string& name) const {
		auto it = factories_.find(name);
		if (it == factories_.end()) {
			return {};
		}

		return it->second->Create();
	}

private:
	std::unordered_map<void*, const char*> types_;
	std::unordered_map<std::string, std::unique_ptr<FactoryBase>> factories_;
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
		Current()[str::kId] = Json::Value(refid);
		Current()[str::kType] = name;
		Select(str::kFields);
		T::AcceptVisitor(value, *this);
	}

	void Add(Ref ref) {
		if (refids_.count(ref) == 0) {
			stack_.insert(ref);
			refids_[ref] = next_refid_++;
		}
	}

	const Json::Value& Write(const char* doctype, int version) {
		StateSentry sentry(this);
		Current()[str::kDoctype] = Json::Value(doctype);
		Current()[str::kVersion] = Json::Value(version);
		Select(str::kObjects) = Json::Value(Json::arrayValue);

		while (!stack_.empty()) {
			StateSentry sentry2(this);
			SelectNext();
			auto p = stack_.begin();
			auto ref = *p;
			stack_.erase(p);
			ref->Write(this);
		}

		return root_;
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
void Serializable<T>::Write(Writer* writer) const {
	writer->Write(static_cast<const T&>(*this));
}


Json::Value Serialize(
	const Registry& reg, Ref ref,
	const char* doctype, int version)
{
	Writer w(reg);
	w.Add(ref);
	return w.Write(doctype, version);
}




class Reader {
	struct State {
		int processed = 0;
		const Json::Value* current;
	};

	class StateSentry {
	public:
		StateSentry(Reader* reader)
			: parent_(reader)
			, state_(reader->state_)
		{}

		~StateSentry() {
			parent_->state_ = state_;
		}

	private:
		Reader* parent_;
		State state_;
	};

public:
	Reader(const Registry& reg, Json::Value root)
		: root_(std::move(root))
		, reg_(reg)
	{
		state_.current = &root_;
	}

	bool ReadHeader() {
		StateSentry sentry(this);
		if (!Current().isObject()) {
			SetError("Invalid document");
			return false;
		}

		if (!Current().isMember(str::kDoctype) ||
			!Current().isMember(str::kVersion) ||
			!Current().isMember(str::kObjects))
		{
			SetError("Missing field in document header");
			return false;
		}

		if (!Current()[str::kDoctype].isString() ||
			!Current()[str::kVersion].isInt() ||
			!Current()[str::kObjects].isArray())
		{
			SetError("Invalid field in document header");
			return false;
		}

		if (Current().size() > 3) {
			SetError("Unexpected field in document header");
			return false;
		}

		return true;
	}

	bool ReadObjects() {
		StateSentry sentry(this);
		Select(str::kObjects);
		if (!Current().isArray() ||
			Current().size() == 0) {
			SetError("Cannot read objects");
			return false;
		}

		int index = -1;
		for (const auto& value : Current()) {
			++index;
			StateSentry sentry2(this);
			Select(value);
			auto result = ReadObject(index);
			if (!result) {
				return false;
			}
		}

		return true;
	}

	bool ReadObject(int index) {
		StateSentry sentry(this);

		if (!Current().isMember(str::kFields) ||
			!Current().isMember(str::kType) ||
			!Current().isMember(str::kId))
		{
			SetError("Missing field in object header");
			return false;
		}

		if (!Current()[str::kFields].isObject() ||
			!Current()[str::kType].isString() ||
			!Current()[str::kId].isInt())
		{
			SetError("Invalid field in object header");
			return false;
		}

		if (Current().size() > 3) {
			SetError("Unexpected field in object header");
			return false;
		}

		auto type = Current()[str::kType].asString();
		auto id = Current()[str::kId].asInt();

		if (objects_.find(id) != objects_.end()) {
			SetError("Duplicate object id found");
			return false;
		}

		auto obj = reg_.Create(type);
		if (!obj) {
			SetError("Unknown type name in object");
			return false;
		}

		Select(str::kFields);
		auto p = obj.get();
		objects_[id] = std::move(obj);

		if (index == 0) {
			first_id_ = id;
		}
		return p->Read(this);
	}

	template<typename T>
	bool Read(T& value) {
		if (!Current().isObject()) {
			SetError("Invalid object descriptor");
			return false;
		}
		auto input_count = Current().size();

		StateSentry sentry(this);
		state_.processed = 0;
		T::AcceptVisitor(value, *this);

		if (state_.processed < input_count) {
			SetError("Unexpected field in object");
			return false;
		}

		return !failed_;
	}

	template<typename T>
	void VisitField(T& value, const char* name) {
		if (failed_) {
			return;
		}

		if (!Current().isMember(name)) {
			SetError("Missing field");
			return;
		}

		++state_.processed;
		StateSentry sentry(this);
		Select(name);
		VisitValue(value);
	}

	const std::string& GetError() const {
		return error_;
	}

	std::vector<UniqueRef> GetObjects() {
		std::vector<UniqueRef> result;

		auto head = objects_.find(first_id_);
		assert(head != objects_.end() && "Missing first object");

		result.push_back(std::move(head->second));
		objects_.erase(head);

		for (auto& obj : objects_) {
			result.push_back(std::move(obj.second));
		}
		return result;
	}

	bool ResolveRefs() {
		for (auto& x : refs_) {
			auto refp = x.first;
			auto refid = x.second;
			auto it = objects_.find(refid);
			if (it == objects_.end()) {
				SetError("Invalid reference");
				return false;
			}

			*refp = it->second.get();
		}

		return true;
	}

private:
	template<typename T>
	void VisitValue(T& value) {
		typename TypeTag<T>::Type tag;
		VisitValue(value, tag);
	}

	void VisitValue(int& value, PrimitiveTag) {
		if (!Current().isInt()) {
			SetError("Invalid value type - int required");
			return;
		}

		value = Current().asInt();
	}

	void VisitValue(std::string& value, PrimitiveTag) {
		if (!Current().isString()) {
			SetError("Invalid value type - string required");
			return;
		}

		value = Current().asString();
	}

	template<typename T>
	void VisitValue(T& value, ArrayTag) {
		if (!Current().isArray()) {
			SetError("Invalid value type - array required");
			return;
		}

		value.reserve(Current().size());

		for (auto& x : Current()) {
			if (failed_) {
				break;
			}

			StateSentry sentry(this);
			Select(x);
			value.emplace_back();
			VisitValue(value.back());
		}
	}

	template<typename T>
	void VisitValue(T& value, ObjectTag) {
		if (!Current().isObject()) {
			SetError("Invalid value type - object required");
			return;
		}

		auto input_count = Current().size();
		state_.processed = 0;
		T::AcceptVisitor(value, *this);
		if (state_.processed < input_count) {
			SetError("Unexpected field in object");
		}
	}

	template<typename T>
	void VisitValue(T& value, RefTag) {
		if (!Current().isInt()) {
			SetError("Invalid value type - int required");
			return;
		}

		auto refid = Current().asInt();
		refs_.emplace_back(&value, refid);
	}

	void SetError(std::string error) {
		error_ = std::move(error);
		failed_ = true;
	}

	const Json::Value& Current() {
		return *state_.current;
	}

	const Json::Value& Select(const char* name) {
		state_.current = &Current()[name];
		return Current();
	}

	const Json::Value& Select(const Json::Value& value) {
		state_.current = &value;
		return Current();
	}

	std::string doctype_;
	int version_ = 0;

	const Json::Value root_;
	const Registry& reg_;
	State state_;
	std::string error_;
	bool failed_ = false;

	int first_id_ = 0;
	std::unordered_map<int, UniqueRef> objects_;
	std::vector<std::pair<Ref*, int>> refs_;
};


template<typename T>
bool Serializable<T>::Read(Reader* reader) {
	return reader->Read(static_cast<T&>(*this));
}


std::vector<UniqueRef> Deserialize(const Registry& reg, Json::Value root) {
	Reader reader(reg, std::move(root));
	auto result =
		reader.ReadHeader() &&
		reader.ReadObjects() &&
		reader.ResolveRefs();

	if (!result) {
		std::cerr << "error: " << reader.GetError() << std::endl;
		return {};
	}

	return reader.GetObjects();
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


struct PolyLine : Serializable<PolyLine> {
	Array<Point> points;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.points, "points");
	}
};


struct Group : Serializable<Group> {
	Array<Ref> elements;
	std::string name;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.elements, "elements");
		v.VisitField(self.name, "name");
	}
};


void CheckSerial() {
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
	g2.name = "g1";

	g1.elements.push_back(&c1);
	g1.elements.push_back(&s1);
	g1.elements.push_back(&c2);
	g1.elements.push_back(&c3);
	g1.elements.push_back(&g2);
	g2.elements.push_back(&s2);
	g2.elements.push_back(&p1);

	Json::Value root = Serialize(reg, &g1, "sample", 1);

	// Dump(root);
	auto objs = Deserialize(reg, root);

	if (!objs.empty()) {
		Json::Value root2 = Serialize(reg, objs[0].get(), "sample2", 1);
		Dump(root2);
	}
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
