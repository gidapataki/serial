#pragma once
#include <unordered_map>
#include <unordered_set>
#include "serial/SerialFwd.h"
#include "serial/TypeTraits.h"
#include "serial/DocInfo.h"
#include "serial/Constants.h"
#include "jsoncpp/json.h"


namespace serial {

class Writer {
public:
	Writer(const Registry& reg);
	ErrorCode Write(const Header& header, Ref ref, Json::Value& output);

	template<typename T> void VisitField(const T& value, const char* name);
	template<typename T> void WriteReferable(const T& value);

private:
	class StateSentry {
	public:
		StateSentry(Writer* writer);
		~StateSentry();

	private:
		Writer* writer_;
		Json::Value* current_;
	};

	void Add(Ref ref);

	Json::Value& Select(const char* name);
	Json::Value& SelectNext();
	Json::Value& Current();

	template<typename T> void VisitValue(const T& value);
	template<typename T> void VisitValue(const T& value, RefTag);
	template<typename T> void VisitValue(const T& value, PrimitiveTag);
	template<typename T> void VisitValue(const T& value, ArrayTag);
	template<typename T> void VisitValue(const T& value, ObjectTag);
	template<typename T> void VisitValue(const T& value, EnumTag);

	const Registry& reg_;
	ErrorCode error_ = ErrorCode::kNone;
	int next_refid_ = 0;
	bool enable_asserts_ = true;

	std::unordered_map<Ref, int> refids_;
	std::unordered_set<Ref> remaining_refs_;

	Json::Value root_;
	Json::Value* current_ = &root_;
};

} // namespace serial

#include "serial/Writer-inl.h"
