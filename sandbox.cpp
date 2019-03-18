#include <iostream>
#include <type_traits>
#include <map>



template<typename E>
class Enum {
public:
	using value_type = E;

	Enum() = default;
	Enum(value_type v) : value_(v) {}

	value_type& operator=(value_type v) { value_ = v; return value_; }
	operator value_type() { return value_; }

	friend bool operator==(Enum lhs, value_type rhs) {
		return lhs.value_ == rhs;
	}

	friend bool operator==(value_type lhs, Enum rhs) {
		return lhs == rhs.value_;
	}

	friend bool operator!=(Enum lhs, value_type rhs) {
		return lhs.value_ != rhs;
	}

	friend bool operator!=(value_type lhs, Enum rhs) {
		return lhs != rhs.value_;
	}

	bool operator==(Enum other) const {
		return value_ == other.value_;
	}

	bool operator!=(Enum other) const {
		return value_ != other.value_;
	}

private:
	value_type value_;
};



namespace wx {

enum class Winding {
	kClockwise,
	kCounterClockwise
};

template<typename Visitor>
static void AcceptVisitor(Visitor& v, Winding) {
	v.VisitValue(Winding::kClockwise, "cw");
	v.VisitValue(Winding::kCounterClockwise, "ccw");
}

} // namespace wx

namespace vis {

struct Vis {
	template<typename E>
	void Register() {
		typename E::value_type v;
		AcceptVisitor(*this, v);
	}

	template<typename E>
	void VisitValue(E value, const char* name) {
		static_assert(std::is_enum<E>::value, "Only enums are allowed");
		static_assert(std::is_same<int, typename std::underlying_type<E>::type>::value, "Invalid underlying_type");
		to_string[int(value)] = name;
		to_value[name] = int(value);
	}

	std::map<int, std::string> to_string;
	std::map<std::string, int> to_value;
};

} // namespace

int main() {
	vis::Vis v;
	v.Register<Enum<wx::Winding>>();

	// std::cerr << v.to_string[1] << std::endl;
	Enum<wx::Winding> e;
	e = wx::Winding::kCounterClockwise;
}
