#include "gtest/gtest.h"
#include "serial/Registry.h"
#include "serial/Referable.h"
#include <map>

using namespace serial;

namespace {

struct A : Referable<A> {
	static constexpr auto kTypeName = "a";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct A2 : Referable<A2> {
	static constexpr auto kTypeName = "a";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct B : Referable<B> {
	static constexpr auto kTypeName = "b";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct C : Referable<C> {
	static constexpr auto kTypeName = "c";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct X : Referable<X> {
	static constexpr auto kTypeName ="x";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

// const char* X::kTypeName;

enum class Index {
	kOne,
	kTwo,
	kRed,
	kRed2,
	kGreen,
	kBlue,
};

std::map<Index, const char*> enum_name;
std::map<Index, bool> enum_enabled;

void Reset(Index i) {
	enum_name[i] = nullptr;
	enum_enabled[i] = false;
}

void Enable(Index i, const char* name) {
	enum_enabled[i] = true;
	enum_name[i] = name;
}

bool IsEnabled(Index i) {
	return enum_enabled[i];
}

const char* Name(Index i) {
	return enum_name[i];
}

struct E : Enum {
	enum Value : int {
		kOne,
		kTwo,
	} value = {};

	E() = default;
	E(Value v) : value(v) {}

	static constexpr auto kTypeName = "E";

	template<typename V>
	static void AcceptVisitor(V& v) {
		if (IsEnabled(Index::kOne)) { v.VisitValue(kOne, Name(Index::kOne)); }
		if (IsEnabled(Index::kTwo)) { v.VisitValue(kTwo, Name(Index::kTwo)); }
	}

	static void Reset() {
		::Reset(Index::kOne);
		::Reset(Index::kTwo);
	}
};

struct F : Enum {
	enum SomeOtherName : int {
		kRed,
		kGreen,
		kBlue,
	} value = {};

	F() = default;
	F(SomeOtherName v) : value(v) {}

	static constexpr auto kTypeName = "F";

	template<typename V>
	static void AcceptVisitor(V& v) {
		if (IsEnabled(Index::kRed)) { v.VisitValue(kRed, Name(Index::kRed)); }
		if (IsEnabled(Index::kRed2)) { v.VisitValue(kRed, Name(Index::kRed2)); }
		if (IsEnabled(Index::kGreen)) { v.VisitValue(kGreen, Name(Index::kGreen)); }
		if (IsEnabled(Index::kBlue)) { v.VisitValue(kBlue, Name(Index::kBlue)); }
	}

	static void Reset() {
		::Reset(Index::kRed);
		::Reset(Index::kRed2);
		::Reset(Index::kGreen);
		::Reset(Index::kBlue);
	}
};

struct G : Enum {
	E::Value value = {};

	G() = default;
	G(E::Value v) : value(v) {}

	static constexpr auto kTypeName = "G";

	template<typename V>
	static void AcceptVisitor(V& v) {
		v.VisitValue(E::Value::kOne, "one");
		v.VisitValue(E::Value::kTwo, "two");
	}
};


} // namespace

TEST(RegistryTest, Register) {
	Registry reg(noasserts);

	EXPECT_TRUE(reg.Register<A>());
	EXPECT_FALSE(reg.Register<A2>());
	EXPECT_TRUE(reg.Register<B>());

	// X::kTypeName = "x1";
	// EXPECT_TRUE(reg.Register<X>());

	// X::kTypeName = "x2";
	// EXPECT_FALSE(reg.Register<X>());
}

TEST(RegistryTest, Scoped) {
	Registry reg1(noasserts);
	Registry reg2(noasserts);

	EXPECT_TRUE(reg1.Register<A>());
	EXPECT_TRUE(reg1.Register<B>());

	EXPECT_TRUE(reg2.Register<A>());
	EXPECT_FALSE(reg2.Register<A2>());
	EXPECT_TRUE(reg2.Register<C>());
}

TEST(RegistryTest, Create) {
	Registry reg(noasserts);

	reg.Register<A>();
	reg.Register<B>();

	auto a = reg.CreateReferable("a");
	auto b = reg.CreateReferable("b");
	auto c = reg.CreateReferable("c");

	EXPECT_NE(nullptr, a);
	EXPECT_NE(nullptr, b);
	EXPECT_EQ(nullptr, c);

	EXPECT_EQ(StaticTypeId<A>::Get(), a->GetTypeId());
	EXPECT_EQ(StaticTypeId<B>::Get(), b->GetTypeId());
}

TEST(RegistryTest, RegisterNullptr) {
	Registry reg(noasserts);

	E::Reset();
	Enable(Index::kOne, nullptr);
	Enable(Index::kTwo, "two");
	EXPECT_FALSE(reg.Register<E>());

	E::Reset();
	Enable(Index::kTwo, "two");
	EXPECT_TRUE(reg.Register<E>());
}

TEST(RegistryTest, RegisterEnum) {
	Registry reg(noasserts);

	E::Reset();
	EXPECT_TRUE(reg.Register<E>());
	EXPECT_TRUE(reg.Register<E>());

	F::Reset();
	Enable(Index::kRed, "red");
	Enable(Index::kRed2, "not_red");
	EXPECT_FALSE(reg.Register<F>());

	F::Reset();
	Enable(Index::kRed, "red");
	Enable(Index::kGreen, "red");
	EXPECT_FALSE(reg.Register<F>());

	F::Reset();
	Enable(Index::kRed, "red");
	Enable(Index::kGreen, "green");
	EXPECT_TRUE(reg.Register<F>());

	F::Reset();
	Enable(Index::kBlue, "blue");
	EXPECT_TRUE(reg.IsRegistered<F>());
	EXPECT_TRUE(reg.Register<F>());
	EXPECT_EQ(nullptr, reg.EnumToString(F{F::kBlue}));
}

TEST(RegistryTest, EnumToString) {
	Registry reg(noasserts);

	E::Reset();
	F::Reset();
	Enable(Index::kRed, "red");
	Enable(Index::kBlue, "blue");
	reg.Register<F>();

	EXPECT_EQ(nullptr, reg.EnumToString(E{E::kOne}));
	EXPECT_EQ(nullptr, reg.EnumToString(E{E::kTwo}));
	EXPECT_EQ(nullptr, reg.EnumToString(F{F::kGreen}));

	EXPECT_EQ(std::string{"red"}, reg.EnumToString(F{F::kRed}));
	EXPECT_EQ(std::string{"blue"}, reg.EnumToString(F{F::kBlue}));

	E::Reset();
	Enable(Index::kOne, "one");

	EXPECT_TRUE(reg.Register<E>());
	EXPECT_EQ(std::string{"one"}, reg.EnumToString(E{E::kOne}));
	EXPECT_EQ(nullptr, reg.EnumToString(E{E::kTwo}));
}

TEST(RegistryTest, EnumIsScoped) {
	Registry reg1(noasserts);
	Registry reg2(noasserts);

	E::Reset();
	Enable(Index::kOne, "1");
	reg1.Register<E>();

	E::Reset();
	Enable(Index::kOne, "one");
	Enable(Index::kTwo, "two");
	reg2.Register<E>();

	EXPECT_EQ(std::string{"1"}, reg1.EnumToString(E{E::kOne}));
	EXPECT_EQ(nullptr, reg1.EnumToString(E{E::kTwo}));

	EXPECT_EQ(std::string{"one"}, reg2.EnumToString(E{E::kOne}));
	EXPECT_EQ(std::string{"two"}, reg2.EnumToString(E{E::kTwo}));
}

TEST(RegistryTest, EnumFromString) {
	Registry reg(noasserts);

	F f_value = F::kGreen;
	EXPECT_FALSE(reg.EnumFromString("first", f_value));
	EXPECT_EQ(F::kGreen, f_value.value);

	E::Reset();
	F::Reset();

	Enable(Index::kOne, "1");
	Enable(Index::kTwo, "first");
	Enable(Index::kRed, "first");
	Enable(Index::kBlue, "blue");

	reg.Register<E>();
	reg.Register<F>();

	EXPECT_TRUE(reg.EnumFromString("first", f_value));
	EXPECT_EQ(F::kRed, f_value.value);

	f_value = F::kBlue;
	EXPECT_FALSE(reg.EnumFromString("???", f_value));
	EXPECT_EQ(F::kBlue, f_value.value);

	f_value = F::kGreen;
	EXPECT_FALSE(reg.EnumFromString("???", f_value));
	EXPECT_EQ(F::kGreen, f_value.value);

	EXPECT_TRUE(reg.EnumFromString("blue", f_value));
	EXPECT_EQ(F::kBlue, f_value.value);

	E e_value = E::kTwo;
	EXPECT_TRUE(reg.EnumFromString("1", e_value));
	EXPECT_EQ(E::kOne, e_value.value);

	EXPECT_TRUE(reg.EnumFromString("first", e_value));
	EXPECT_EQ(E::kTwo, e_value.value);
}

TEST(RegistryTest, SameEnumType) {
	Registry reg(noasserts);

	E::Reset();
	Enable(Index::kTwo, "2");
	EXPECT_TRUE(reg.Register<E>());
	EXPECT_TRUE(reg.Register<G>());

	EXPECT_EQ(nullptr, reg.EnumToString(E{E::kOne}));
	EXPECT_EQ(std::string{"one"}, reg.EnumToString(G{E::kOne}));

	EXPECT_EQ(std::string{"2"}, reg.EnumToString(E{E::kTwo}));
	EXPECT_EQ(std::string{"two"}, reg.EnumToString(G{E::kTwo}));
}
