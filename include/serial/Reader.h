#pragma once
#include <unordered_map>
#include <vector>
#include "serial/SerialFwd.h"
#include "serial/Constants.h"
#include "serial/TypeTraits.h"
#include "serial/DocInfo.h"
#include "jsoncpp/json.h"


namespace serial {

class Reader {
public:
	Reader(const Json::Value& root);

	ErrorCode ReadHeader(Header& header);
	ErrorCode ReadObjects(const Registry& reg, std::vector<UniqueRef>& refs);

	template<typename T> void ReadReferable(T& value);
	template<typename T> void VisitField(T& value, const char* name);

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

	void ReadObjectsInternal(const Registry& reg);
	void ReadObjectInternal(int index, const Registry& reg);
	bool ResolveRefs();
	void ExtractRefs(std::vector<UniqueRef>& refs);

	template<typename T> void VisitValue(T& value);
	template<typename T> void VisitValue(T& value, ArrayTag);
	template<typename T> void VisitValue(T& value, ObjectTag);
	template<typename T> void VisitValue(T& value, EnumTag);

	void VisitValue(int& value, PrimitiveTag);
	void VisitValue(std::string& value, PrimitiveTag);
	void VisitValue(Ref& value, RefTag);

	void SetError(ErrorCode error);
	bool IsError() const;

	const Json::Value& Current();
	const Json::Value& Select(const char* name);
	const Json::Value& Select(const Json::Value& value);

	const Json::Value& root_;
	const Registry* reg_ = nullptr;
	State state_;
	ErrorCode error_;

	int root_id_ = 0;
	std::unordered_map<int, UniqueRef> objects_; // todo rename?
	std::vector<std::pair<Ref*, int>> unresolved_refs_;
};

} // namespace serial

#include "serial/Reader-inl.h"
