#pragma once
#include <unordered_map>
#include <vector>
#include "serial/SerialFwd.h"
#include "serial/Constants.h"
#include "serial/TypeTraits.h"
#include "serial/Header.h"
#include "serial/Version.h"
#include "jsoncpp/json.h"


namespace serial {

class Reader {
public:
	Reader(const Json::Value& root);

	ErrorCode ReadHeader(Header& header);
	ErrorCode ReadObjects(
		const Registry& reg, RefContainer& refs, ReferableBase*& root);

	template<typename T> void ReadReferable(T& value);
	template<typename T> void ReadVariant(T& value);

	template<typename T> void VisitField(T& value, const char* name, MinVersion = {}, MaxVersion = {});

	bool IsVersionInRange(const MinVersion& v0, const MaxVersion& v1) const; // fixme - private
	void SetError(ErrorCode error); // fixme - private

private:
	struct State {
		int processed = 0;
		const Json::Value* current;
	};

	class StateSentry {
	public:
		StateSentry(Reader* reader);
		~StateSentry();

	private:
		Reader* reader_;
		State state_;
	};

	template<typename V, typename... Us>
	struct ForEachVariantType;

	void ReadObjectsInternal(const Registry& reg);
	void ReadObjectInternal(const Registry& reg);
	void ResolveRefs();
	void ExtractRefs(RefContainer& refs, ReferableBase*& root);
	bool CheckVariant();

	template<typename T> void VisitValue(T& value);
	template<typename T> void VisitValue(T& value, ArrayTag);
	template<typename T> void VisitValue(T& value, OptionalTag);
	template<typename T> void VisitValue(T& value, ObjectTag);
	template<typename T> void VisitValue(T& value, EnumTag);
	template<typename T> void VisitValue(T& value, RefTag);
	template<typename T> void VisitValue(T& value, UserTag);
	template<typename T> void VisitValue(T& value, VariantTag);

	void VisitValue(bool& value, PrimitiveTag);
	void VisitValue(int& value, PrimitiveTag);
	void VisitValue(int64_t& value, PrimitiveTag);
	void VisitValue(unsigned& value, PrimitiveTag);
	void VisitValue(uint64_t& value, PrimitiveTag);
	void VisitValue(float& value, PrimitiveTag);
	void VisitValue(double& value, PrimitiveTag);
	void VisitValue(std::string& value, PrimitiveTag);

	bool IsError() const;

	const Json::Value& Current();
	const Json::Value& Select(const char* name);
	const Json::Value& Select(const Json::Value& value);

	const Json::Value& root_;
	const Registry* reg_ = nullptr;
	State state_;
	ErrorCode error_;
	int version_ = 0;

	using RefId = std::string;

	RefId root_id_ = {};
	std::unordered_map<RefId, UniqueRef> objects_;
	std::vector<std::pair<RefBase*, RefId>> unresolved_refs_;
};

} // namespace serial

#include "serial/Reader-inl.h"
