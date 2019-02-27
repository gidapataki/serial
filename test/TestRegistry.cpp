#include "gtest/gtest.h"
#include "serial/Registry.h"
#include "serial/Referable.h"

using namespace serial;

namespace {

struct A : Referable<A> {
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct B : Referable<B> {
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct C : Referable<C> {
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

enum class E {
	kOne,
	kTwo,
};

enum class F {
	kRed,
	kGreen,
	kBlue,
};

} // namespace

TEST(RegistryTest, Register) {
	Registry reg(noasserts);

	EXPECT_TRUE(reg.Register<A>("A"));
	EXPECT_FALSE(reg.Register<A>("B"));
	EXPECT_FALSE(reg.Register<B>("A"));
	EXPECT_TRUE(reg.Register<B>("B"));
	EXPECT_FALSE(reg.Register<A>("B"));
	EXPECT_FALSE(reg.Register<B>("C"));
}

TEST(RegistryTest, Scoped) {
	Registry reg1(noasserts);
	Registry reg2(noasserts);

	EXPECT_TRUE(reg1.Register<A>("A"));
	EXPECT_TRUE(reg2.Register<A>("B"));

	EXPECT_TRUE(reg1.Register<B>("B"));
	EXPECT_FALSE(reg2.Register<B>("B"));
	EXPECT_TRUE(reg2.Register<B>("B2"));
}

TEST(RegistryTest, Name) {
	Registry reg(noasserts);

	EXPECT_EQ(nullptr, reg.GetName<A>());
	EXPECT_EQ(nullptr, reg.GetName<B>());

	reg.Register<A>("TypeA");
	reg.Register<B>("TypeB");

	EXPECT_EQ(std::string{"TypeA"}, reg.GetName<A>());
	EXPECT_EQ(std::string{"TypeB"}, reg.GetName<B>());
	EXPECT_EQ(nullptr, reg.GetName<C>());
}

TEST(RegistryTest, Create) {
	Registry reg(noasserts);

	reg.Register<A>("A");
	reg.Register<B>("B");

	auto a = reg.Create("A");
	auto b = reg.Create("B");
	auto c = reg.Create("C");

	EXPECT_NE(nullptr, a);
	EXPECT_NE(nullptr, b);
	EXPECT_EQ(nullptr, c);

	EXPECT_EQ(StaticTypeId<A>::Get(), a->GetTypeId());
	EXPECT_EQ(StaticTypeId<B>::Get(), b->GetTypeId());
}

TEST(RegistryTest, RegisterEnum) {
	Registry reg(noasserts);

	EXPECT_TRUE(reg.RegisterEnum<E>({}));
	EXPECT_FALSE(reg.RegisterEnum<E>({}));

	EXPECT_FALSE(reg.RegisterEnum<F>({{F::kRed, "red"}, {F::kRed, "green"}}));
	EXPECT_FALSE(reg.RegisterEnum<F>({{F::kRed, "red"}, {F::kGreen, "red"}}));
	EXPECT_TRUE(reg.RegisterEnum<F>({{F::kRed, "red"}, {F::kGreen, "green"}}));
	EXPECT_FALSE(reg.RegisterEnum<F>({{F::kBlue, "blue"}}));
}

TEST(RegistryTest, EnumToString) {
	Registry reg(noasserts);

	reg.RegisterEnum<F>({{F::kRed, "red"}, {F::kBlue, "blue"}});

	EXPECT_EQ(nullptr, reg.EnumToString(E::kOne));
	EXPECT_EQ(nullptr, reg.EnumToString(E::kTwo));
	EXPECT_EQ(nullptr, reg.EnumToString(F::kGreen));

	EXPECT_EQ(std::string{"red"}, reg.EnumToString(F::kRed));
	EXPECT_EQ(std::string{"blue"}, reg.EnumToString(F::kBlue));

	EXPECT_TRUE(reg.RegisterEnum<E>({{E::kOne, "one"}}));
	EXPECT_EQ(std::string{"one"}, reg.EnumToString(E::kOne));
	EXPECT_EQ(nullptr, reg.EnumToString(E::kTwo));
}

TEST(RegistryTest, EnumIsScoped) {
	Registry reg1(noasserts);
	Registry reg2(noasserts);

	reg1.RegisterEnum<E>({{E::kOne, "1"}});
	reg2.RegisterEnum<E>({{E::kOne, "one"}, {E::kTwo, "two"}});

	EXPECT_EQ(std::string{"1"}, reg1.EnumToString(E::kOne));
	EXPECT_EQ(nullptr, reg1.EnumToString(E::kTwo));

	EXPECT_EQ(std::string{"one"}, reg2.EnumToString(E::kOne));
	EXPECT_EQ(std::string{"two"}, reg2.EnumToString(E::kTwo));
}

TEST(RegistryTest, EnumFromString) {
	Registry reg(noasserts);

	F f_value = F::kGreen;
	EXPECT_FALSE(reg.EnumFromString("first", f_value));
	EXPECT_EQ(F::kGreen, f_value);

	reg.RegisterEnum<E>({{E::kOne, "1"}, {E::kTwo, "first"}});
	reg.RegisterEnum<F>({{F::kRed, "first"}, {F::kBlue, "blue"}});


	EXPECT_TRUE(reg.EnumFromString("first", f_value));
	EXPECT_EQ(F::kRed, f_value);

	f_value = F::kBlue;
	EXPECT_FALSE(reg.EnumFromString("???", f_value));
	EXPECT_EQ(F::kBlue, f_value);

	f_value = F::kGreen;
	EXPECT_FALSE(reg.EnumFromString("???", f_value));
	EXPECT_EQ(F::kGreen, f_value);

	EXPECT_TRUE(reg.EnumFromString("blue", f_value));
	EXPECT_EQ(F::kBlue, f_value);

	E e_value = E::kTwo;
	EXPECT_TRUE(reg.EnumFromString("1", e_value));
	EXPECT_EQ(E::kOne, e_value);

	EXPECT_TRUE(reg.EnumFromString("first", e_value));
	EXPECT_EQ(E::kTwo, e_value);
}

