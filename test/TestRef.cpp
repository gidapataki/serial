#include "gtest/gtest.h"
#include "serial/Ref.h"
#include "serial/Referable.h"


using namespace serial;

namespace {

struct A : Referable<A> {
	int value = 5;

	static constexpr auto kTypeName = "a";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct B : Referable<B> {
	std::string value = "hello";

	static constexpr auto kTypeName = "b";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct C : Referable<C> {
	static constexpr auto kTypeName = "c";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

} // namespace

TEST(TypedRefTest, Ctor) {
	A a;
	B b;
	Ref<A> ref1;
	Ref<A> ref2(nullptr);
	Ref<A> ref3(&a);

	EXPECT_EQ(nullptr, ref1.Get());
	EXPECT_EQ(nullptr, ref2.Get());
	EXPECT_EQ(&a, ref3.Get());

	const auto& ref4 = ref3;
	EXPECT_EQ(&a, ref4.Get());

	Ref<A, B> ref5(&b);
	Ref<A, B> ref6(ref5);

	EXPECT_EQ(&b, ref5.Get());
	EXPECT_EQ(&b, ref6.Get());

	Ref<A, B> ref7(std::move(ref6));
	EXPECT_EQ(&b, ref7.Get());
}

TEST(TypedRefTest, Assign) {
	A a;
	B b;

	Ref<A, B> ref1;
	Ref<A, B> ref2;

	ref1 = &a;
	ref2 = &b;
	EXPECT_EQ(&a, ref1.Get());
	EXPECT_EQ(&b, ref2.Get());

	ref1 = nullptr;
	EXPECT_EQ(nullptr, ref1.Get());

	ref1 = ref2;
	EXPECT_EQ(&b, ref1.Get());
}

TEST(TypedRefTest, Eq) {
	A a;
	B b;

	Ref<A, B> ref0;
	Ref<A, B> ref1;
	Ref<A, B> ref2(&b);

	EXPECT_FALSE(ref1);
	EXPECT_TRUE(ref2);

	EXPECT_TRUE(ref0 == ref1);
	EXPECT_FALSE(ref0 == ref2);
	EXPECT_FALSE(ref1 == ref2);
	EXPECT_FALSE(ref0 != ref1);

	ref1 = &b;
	EXPECT_TRUE(ref1 == ref2);
	EXPECT_FALSE(ref1 != ref2);
	EXPECT_TRUE(ref0 != ref1);

	ref1 = &a;
	EXPECT_FALSE(ref1 == ref2);
}

TEST(TypedRefTest, IsAs) {
	A a;
	B b;

	Ref<A, B> ref1;
	Ref<A, B> ref2;

	EXPECT_FALSE(ref1.Is<A>());
	EXPECT_FALSE(ref2.Is<A>());

	ref1 = &a;
	ref2 = &b;

	EXPECT_TRUE(ref1.Is<A>());
	EXPECT_FALSE(ref2.Is<A>());
	EXPECT_FALSE(ref1.Is<B>());
	EXPECT_TRUE(ref2.Is<B>());

	EXPECT_EQ(&a, &ref1.As<A>());
	EXPECT_EQ(&b, &ref2.As<B>());

	const Ref<A, B> ref3 = ref2;
	const Ref<A, B> ref4 = ref1;

	EXPECT_EQ(&b, &ref3.As<B>());
	EXPECT_EQ(&a, &ref4.As<A>());
}

TEST(TypedRefTest, Set) {
	A a;
	B b;
	C c;

	Ref<A, B> ref1;

	EXPECT_TRUE(ref1.Set(nullptr));
	EXPECT_FALSE(ref1.Set(&c));
	EXPECT_EQ(nullptr, ref1.Get());

	EXPECT_TRUE(ref1.Set(&a));
	EXPECT_EQ(&a, ref1.Get());

	EXPECT_FALSE(ref1.Set(&c));
	EXPECT_EQ(&a, ref1.Get());

	EXPECT_TRUE(ref1.Set(nullptr));
	EXPECT_EQ(nullptr, ref1.Get());
}
