#include "gtest/gtest.h"
#include "serial/Referable.h"
#include "serial/MetaHelpers.h"
#include <type_traits>


using namespace serial;
using namespace serial::detail;

namespace {

struct A {};
struct B {};
struct C : Referable<C> {
	static constexpr auto kTypeName = "c";

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {}
};

} // namespace


TEST(MetaTest, Head) {
	EXPECT_TRUE((std::is_same<A, typename Head<Typelist<A>>::Type>::value));
	EXPECT_TRUE((std::is_same<A, typename Head<Typelist<A, B>>::Type>::value));
	EXPECT_FALSE((std::is_same<A, typename Head<Typelist<B, A>>::Type>::value));
}

TEST(MetaTest, OneOf) {
	// Note: some linker error with EXPECT_TRUE without the explicit conversion
	EXPECT_FALSE(bool(IsOneOf<A, Typelist<>>::value));
	EXPECT_FALSE(bool(IsOneOf<A, Typelist<B>>::value));
	EXPECT_TRUE(bool(IsOneOf<A, Typelist<A>>::value));
	EXPECT_FALSE(bool(IsOneOf<A, Typelist<B>>::value));
	EXPECT_FALSE(bool(IsOneOf<A, Typelist<B>>::value));
	EXPECT_TRUE(bool(IsOneOf<A, Typelist<B, C, A>>::value));
}

TEST(MetaTest, IsReferable) {
	EXPECT_FALSE(bool(detail::IsReferable<Typelist<A>>::value));
	EXPECT_TRUE(bool(detail::IsReferable<Typelist<C>>::value));
	EXPECT_FALSE(bool(detail::IsReferable<Typelist<A, C>>::value));
	EXPECT_TRUE(bool(detail::IsReferable<Typelist<C, C>>::value));
}

TEST(MetaTest, IndexOf) {
	EXPECT_EQ(-1, int(IndexOf<A, Typelist<>>::value));
	EXPECT_EQ(0, int(IndexOf<A, Typelist<A>>::value));
	EXPECT_EQ(-1, int(IndexOf<A, Typelist<B>>::value));
	EXPECT_EQ(0, int(IndexOf<A, Typelist<A, B>>::value));
	EXPECT_EQ(2, int(IndexOf<A, Typelist<B, C, A>>::value));
	EXPECT_EQ(2, int(IndexOf<A, Typelist<B, C, A, C, A>>::value));
	EXPECT_EQ(-1, int(IndexOf<A, Typelist<B, C, B>>::value));
}
