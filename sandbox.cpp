#include <iostream>
#include <type_traits>
#include <map>
#include <functional>
#include "serial/Variant.h"


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

void CheckEnum() {
	vis::Vis v;
	v.Register<Enum<wx::Winding>>();

	// std::cerr << v.to_string[1] << std::endl;
	Enum<wx::Winding> e;
	e = wx::Winding::kCounterClockwise;
}


template<typename T>
void DumpType() {
	std::cerr << __PRETTY_FUNCTION__ << std::endl;
}

template<typename... Ts>
struct InVersion;

template<typename T, typename V>
struct InVersion<T(V)> {
	static void DumpIf(std::function<bool(int)> fn) {
		if (fn(V::value)) {
			DumpType<T>();
		}
	}
};

template<typename T, typename... Ts>
struct InVersion<T, Ts...> {
	static void DumpIf(std::function<bool(int)> fn) {
		InVersion<T>::DumpIf(fn);
		InVersion<Ts...>::DumpIf(fn);
	}
};


template<int N>
struct Version {
	static constexpr int value = N;
};


template<typename... Ts>
struct List {};


using Version1 = Version<1>;
using Version2 = Version<2>;


void CheckVersion() {
	List<int(Version1), bool(Version2)> ls;

	InVersion<int(Version1), bool(Version2)>::DumpIf([](int n) -> bool {
		return n > 0;
	});
}


int main() {
	serial::variant::Variant<int, std::string> vs;

	vs = 5;
	std::cout << vs.Get<int>() << std::endl;

	vs = std::string{"hello"};
	std::cout << vs.Get<std::string>() << std::endl;
}
