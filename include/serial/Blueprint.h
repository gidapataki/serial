#pragma once
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include "serial/SerialFwd.h"
#include "serial/TypeTraits.h"
#include "serial/TypeId.h"
#include "serial/Version.h"


namespace serial {

class Blueprint {
public:
	Blueprint() = default;

	static Blueprint FromString(const std::string& str);
	template<typename T> static Blueprint FromType(int version);

	bool AddLine(const std::string& line);

	friend std::string Diff(const Blueprint& lhs, const Blueprint& rhs);
	friend std::ostream& operator<<(std::ostream& stream, const Blueprint& bp);

private:
	std::set<std::string> lines_;
};


class BlueprintWriter {
public:
	explicit BlueprintWriter(Blueprint& bp, int version = 0);

	template<typename T> void Add();
	template<typename T> void VisitField(const T& value, const char* name, BeginVersion v0 = {}, EndVersion v1 = {});
	template<typename T> void VisitEnumValue(const T& value, const char* name, BeginVersion v0 = {}, EndVersion v1 = {});
	template<typename T> void VisitVersionedType(BeginVersion v0, EndVersion v1);

private:
	template<typename T> void AddTypeName(const char* info);
	template<typename T> void Add(ReferableTag);
	template<typename T> void Add(EnumTag);
	template<typename T> void Add(ObjectTag);

	template<typename T> void VisitValue(const T& value);
	template<typename T> void VisitValue(const T& value, PrimitiveTag);
	template<typename T> void VisitValue(const Array<T>& value, ArrayTag);
	template<typename T> void VisitValue(const Optional<T>& value, OptionalTag);
	template<typename T> void VisitValue(const T& value, ObjectTag);
	template<typename T> void VisitValue(const T& value, EnumTag);
	template<typename T> void VisitValue(const T& value, VariantTag);

	template<typename T> void VisitValue(const T& value, RefTag);
	template<typename T> void VisitValue(const T& value, UserTag);

private:
	struct State {
		std::string prefix;
	} state_;

	class StateSentry {
	public:
		StateSentry(BlueprintWriter* parent);
		~StateSentry();

	private:
		State state_;
		BlueprintWriter* parent_ = nullptr;
	};

	std::unordered_set<TypeId> visited_;
	Blueprint& blueprint_;
	int version_ = 0;
};

} // namespace serial

#include "serial/Blueprint-inl.h"
