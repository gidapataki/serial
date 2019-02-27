#include "gtest/gtest.h"
#include "serial/BasicRef.h"
#include "serial/Referable.h"


using namespace serial;

namespace {

struct A : Referable<A> {
	int value = 5;
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct B : Referable<B> {
	std::string value = "hello";
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

struct C : Referable<C> {
	template<typename S, typename V> static void AcceptVisitor(S&, V&) {}
};

} // namespace


TEST(BasicRefTest, Ctor) {
	A a;
	B b;

	BasicRef ref1;
	BasicRef ref2(nullptr);
	BasicRef ref3(&a);

	EXPECT_EQ(nullptr, ref1.Get());
	EXPECT_EQ(nullptr, ref2.Get());
	EXPECT_EQ(&a, ref3.Get());

	const BasicRef& ref4 = ref3;
	EXPECT_EQ(&a, ref4.Get());

	BasicRef ref5(&b);
	BasicRef ref6(ref5);

	EXPECT_EQ(&b, ref5.Get());
	EXPECT_EQ(&b, ref6.Get());

	BasicRef ref7(std::move(ref6));
	EXPECT_EQ(&b, ref7.Get());
}

TEST(BasicRefTest, Assign) {
	A a;
	B b;

	BasicRef ref1;
	BasicRef ref2;

	ref1 = &a;
	ref2 = &b;
	EXPECT_EQ(&a, ref1.Get());
	EXPECT_EQ(&b, ref2.Get());

	ref1 = nullptr;
	EXPECT_EQ(nullptr, ref1.Get());

	ref1 = ref2;
	EXPECT_EQ(&b, ref1.Get());
}

TEST(BasicRefTest, Eq) {
	A a;
	B b;

	BasicRef ref1;
	BasicRef ref2(&b);

	EXPECT_FALSE(ref1);
	EXPECT_TRUE(ref2);

	EXPECT_TRUE(BasicRef{} == ref1);
	EXPECT_FALSE(BasicRef{} == ref2);
	EXPECT_FALSE(ref1 == ref2);
	EXPECT_FALSE(BasicRef{} != ref1);

	ref1 = &b;
	EXPECT_TRUE(ref1 == ref2);
	EXPECT_FALSE(ref1 != ref2);
	EXPECT_TRUE(BasicRef{} != ref1);

	ref1 = &a;
	EXPECT_FALSE(ref1 == ref2);
}

TEST(BasicRefTest, IsAs) {
	A a;
	B b;

	BasicRef ref1;
	BasicRef ref2;

	EXPECT_FALSE(ref1.Is<A>());
	EXPECT_FALSE(ref2.Is<A>());

	ref1 = &a;
	ref2 = &b;

	EXPECT_TRUE(ref1.Is<A>());
	EXPECT_FALSE(ref2.Is<A>());
	EXPECT_FALSE(ref1.Is<B>());
	EXPECT_TRUE(ref2.Is<B>());

	EXPECT_EQ(&a, ref1.Get<A>());
	EXPECT_EQ(nullptr, ref1.Get<B>());

	EXPECT_EQ(&b, ref2.Get<B>());
	EXPECT_EQ(nullptr, ref2.Get<A>());

	const BasicRef ref3 = ref2;
	const BasicRef ref4 = ref1;

	EXPECT_EQ(&b, ref3.Get<B>());
	EXPECT_EQ(nullptr, ref3.Get<A>());

	EXPECT_EQ(&a, ref4.Get<A>());
	EXPECT_EQ(nullptr, ref4.Get<B>());
}
