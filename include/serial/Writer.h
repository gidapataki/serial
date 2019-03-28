#pragma once
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include "serial/SerialFwd.h"
#include "serial/TypeTraits.h"
#include "serial/Header.h"
#include "serial/Constants.h"
#include "serial/Version.h"
#include "jsoncpp/json.h"


namespace serial {


class Writer {
public:
	Writer(const Registry& reg);
	Writer(const Registry& reg, noasserts_t);

	// Note: Write() should be only called once,
	// as it leaves the object in a non-clear state.
	ErrorCode Write(const Header& header, const ReferableBase* ref, Json::Value& output);

	template<typename T> void WriteReferable(const T& value);
	template<typename T> void WriteVariant(const T& value);

	template<typename T> void VisitField(const T& value, const char* name, MinVersion = {}, MaxVersion = {});

private:
	class StateSentry {
	public:
		StateSentry(Writer* writer);
		~StateSentry();

	private:
		Writer* writer_;
		Json::Value* current_;
	};


	class VariantWriter : public Visitor<> {
	public:
		VariantWriter(Writer* writer);
		template<typename T> void operator()(const T& value, const MinVersion& v0, const MaxVersion& v1) const;

	private:
		Writer* writer_;
	};


	std::string AddRef(const ReferableBase* ref);
	Json::Value& Select(const char* name);
	Json::Value& SelectNext();
	Json::Value& Current();

	bool IsVersionInRange(const MinVersion& v0, const MaxVersion& v1) const;
	void SetError(ErrorCode error);

	template<typename T> void VisitValue(const T& value);
	template<typename T> void VisitValue(const T& value, PrimitiveTag);
	template<typename T> void VisitValue(const T& value, ArrayTag);
	template<typename T> void VisitValue(const T& value, OptionalTag);
	template<typename T> void VisitValue(const T& value, ObjectTag);
	template<typename T> void VisitValue(const T& value, EnumTag);
	template<typename T> void VisitValue(const T& value, RefTag);
	template<typename T> void VisitValue(const T& value, UserTag);
	template<typename T> void VisitValue(const T& value, VariantTag);

	void VisitValue(const float& value, PrimitiveTag);
	void VisitValue(const double& value, PrimitiveTag);

	const Registry& reg_;
	ErrorCode error_ = ErrorCode::kNone;
	int next_refid_ = 0;
	int version_ = 0;
	bool enable_asserts_ = true;

	std::unordered_map<const ReferableBase*, std::string> refids_;
	std::unordered_set<const ReferableBase*> remaining_refs_;
	std::deque<const ReferableBase*> queue_;

	Json::Value root_;
	Json::Value* current_ = &root_;
};

} // namespace serial

#include "serial/Writer-inl.h"
