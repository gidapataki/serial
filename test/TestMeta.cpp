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
	template<typename S, typename V>void AcceptVisitor(S& self, V& v) {}
};

} // namespace


TEST(MetaTest, FirstType) {
	EXPECT_TRUE((std::is_same<A, typename FirstType<A>::type>::value));
	EXPECT_TRUE((std::is_same<A, typename FirstType<A, B>::type>::value));
	EXPECT_FALSE((std::is_same<A, typename FirstType<B, A>::type>::value));
}

TEST(MetaTest, OneOf) {
	// Note: some linker error with EXPECT_TRUE without the explicit conversion
	EXPECT_FALSE(bool(IsOneOf<A>::value));
	EXPECT_FALSE(bool(IsOneOf<A, B>::value));
	EXPECT_TRUE(bool(IsOneOf<A, A>::value));
	EXPECT_FALSE(bool(IsOneOf<A, B>::value));
	EXPECT_FALSE(bool(IsOneOf<A, B>::value));
	EXPECT_TRUE(bool(IsOneOf<A, B, C, A>::value));
}

TEST(MetaTest, IsReferable) {
	EXPECT_FALSE(bool(detail::IsReferable<A>::value));
	EXPECT_TRUE(bool(detail::IsReferable<C>::value));
	EXPECT_FALSE(bool(detail::IsReferable<A, C>::value));
	EXPECT_TRUE(bool(detail::IsReferable<C, C>::value));
}

TEST(MetaTest, IndexOf) {
	EXPECT_EQ(-1, int(IndexOf<A>::value));
	EXPECT_EQ(0, int(IndexOf<A, A>::value));
	EXPECT_EQ(-1, int(IndexOf<A, B>::value));
	EXPECT_EQ(0, int(IndexOf<A, A, B>::value));
	EXPECT_EQ(2, int(IndexOf<A, B, C, A>::value));
	EXPECT_EQ(2, int(IndexOf<A, B, C, A, C, A>::value));
	EXPECT_EQ(-1, int(IndexOf<A, B, C, B>::value));
}

TEST(MetaTest, MatchTypeId) {
	auto id = StaticTypeId<A>::Get();
	EXPECT_TRUE((MatchTypeId<A>::Accept(id)));
	EXPECT_FALSE((MatchTypeId<B>::Accept(id)));
	EXPECT_FALSE((MatchTypeId<B, C>::Accept(id)));
	EXPECT_TRUE((MatchTypeId<B, C, A>::Accept(id)));
}

TEST(MetaTest, IndexOfTypeId) {
	auto id = StaticTypeId<A>::Get();

	EXPECT_EQ(0, (IndexOfTypeId<A>::Get(id)));
	EXPECT_EQ(-1, (IndexOfTypeId<B>::Get(id)));
	EXPECT_EQ(-1, (IndexOfTypeId<B, C>::Get(id)));
	EXPECT_EQ(2, (IndexOfTypeId<B, C, A>::Get(id)));
	EXPECT_EQ(1, (IndexOfTypeId<C, A, B>::Get(id)));
}
