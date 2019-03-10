#pragma once
#include <unordered_map>
#include <unordered_set>
#include "serial/SerialFwd.h"
#include "serial/TypeTraits.h"
#include "serial/Header.h"
#include "serial/Constants.h"
#include "jsoncpp/json.h"


namespace serial {

class Writer {
public:
	Writer(const Registry& reg);
	Writer(const Registry& reg, noasserts_t);

	// Note: Write() should be only called once,
	// as it leaves the object in a non-clear state.
	ErrorCode Write(const Header& header, ReferableBase* ref, Json::Value& output);

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

	std::string AddRef(const ReferableBase* ref);

	Json::Value& Select(const char* name);
	Json::Value& SelectNext();
	Json::Value& Current();
	void SetError(ErrorCode error);

	template<typename T> void VisitValue(const T& value);
	template<typename T> void VisitValue(const T& value, PrimitiveTag);
	template<typename T> void VisitValue(const T& value, ArrayTag);
	template<typename T> void VisitValue(const T& value, OptionalTag);
	template<typename T> void VisitValue(const T& value, ObjectTag);
	template<typename T> void VisitValue(const T& value, EnumTag);
	template<typename T> void VisitValue(const T& value, BasicRefTag);
	template<typename T> void VisitValue(const T& value, TypedRefTag);
	template<typename T> void VisitValue(const T& value, UserTag);

	void VisitValue(const float& value, PrimitiveTag);
	void VisitValue(const double& value, PrimitiveTag);

	const Registry& reg_;
	ErrorCode error_ = ErrorCode::kNone;
	int next_refid_ = 0;
	bool enable_asserts_ = true;

	std::unordered_map<const ReferableBase*, std::string> refids_;
	std::unordered_set<const ReferableBase*> remaining_refs_;

	Json::Value root_;
	Json::Value* current_ = &root_;
};

} // namespace serial

#include "serial/Writer-inl.h"
