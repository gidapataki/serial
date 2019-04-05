#pragma once
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <initializer_list>
#include "serial/SerialFwd.h"
#include "serial/TypeTraits.h"
#include "serial/TypeId.h"
#include "serial/Version.h"


namespace serial {

class Blueprint {
public:
	struct Delta {
		Delta() = default;
		Delta(const Delta& other) = default;
		Delta(const std::string& value);

		std::string value;
	};

	static const Delta kNoDiff;

	Blueprint() = default;

	static Blueprint FromString(const std::string& str);
	template<typename T> static Blueprint FromType(int version = 0);

	bool AddLine(std::string line);
	std::size_t Size() const;

	// Note: comparing Blueprint::Delta in tests shows a nicer error
	// when two Blueprints are different, this is why there is no comparison
	// operators on Blueprint itself.

	friend Delta Diff(const Blueprint& lhs, const Blueprint& rhs);
	friend Blueprint Union(std::initializer_list<Blueprint> bps);
	friend std::ostream& operator<<(std::ostream& stream, const Blueprint& bp);
	friend std::string ToString(const Blueprint& bp);

private:
	std::set<std::string> lines_;
};

Blueprint Union(std::initializer_list<Blueprint> bps);
std::ostream& operator<<(std::ostream& stream, const Blueprint::Delta& delta);
bool operator==(const Blueprint::Delta& lhs, const Blueprint::Delta& rhs);
bool operator!=(const Blueprint::Delta& lhs, const Blueprint::Delta& rhs);


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
	template<typename T> void Add(UserTag);
	template<typename T> void Add(PrimitiveTag);

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
