#pragma once
#include <string>
#include <vector>
#include <set>
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

	template<typename T> void VisitType();

	template<typename T> void VisitField(const T& value, const char* name, BeginVersion v0 = {}, EndVersion v1 = {});
	template<typename T> void VisitEnumValue(const T& value, const char* name, BeginVersion v0 = {}, EndVersion v1 = {});
	template<typename T> void VisitVersionedType(BeginVersion v0, EndVersion v1);

private:
	void Push(const char* str);
	void PushQualifier(const char* str);
	void Add(const char* str);

	template<typename T> void AddType(const char* info);
	template<typename T> void VisitType(ReferableTag);
	template<typename T> void VisitType(EnumTag);
	template<typename T> void VisitType(ObjectTag);
	template<typename T> void VisitType(UserTag);
	template<typename T> void VisitType(PrimitiveTag);

	template<typename T> void VisitInternal();
	template<typename T> void VisitInternal(PrimitiveTag);
	template<typename T> void VisitInternal(ArrayTag);
	template<typename T> void VisitInternal(OptionalTag);
	template<typename T> void VisitInternal(ObjectTag);
	template<typename T> void VisitInternal(EnumTag);
	template<typename T> void VisitInternal(VariantTag);
	template<typename T> void VisitInternal(RefTag);
	template<typename T> void VisitInternal(UserTag);

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

	Blueprint& blueprint_;
	int version_ = 0;
};

} // namespace serial

#include "serial/Blueprint-inl.h"
