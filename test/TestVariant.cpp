#include "gtest/gtest.h"
#include "serial/Serial.h"
#include <unordered_map>

using namespace serial;

namespace {

struct A {
	int value = 5;

	bool operator==(const A& other) const { return value == other.value; }
	bool operator!=(const A& other) const { return value != other.value; }

	static constexpr auto kTypeName = "a";
	template<typename S, typename V>void AcceptVisitor(S& self, V& v) {}
};

struct B {
	bool operator==(const B& other) const { return true; }
	bool operator!=(const B& other) const { return false; }

	static constexpr auto kTypeName = "b";
	template<typename S, typename V>void AcceptVisitor(S& self, V& v) {}
};

struct E : Enum {
	enum Value : int {
		kOne,
		kTwo
	} value;

	E() = default;
	E(Value v) : value(v) {}
	bool operator==(const E& other) const { return value == other.value; }
	bool operator!=(const E& other) const { return value != other.value; }

	static constexpr auto kTypeName = "e";
	template<typename S, typename V>void AcceptVisitor(S& self, V& v) {}
};

struct X {
	X() = default;
	X(const X&) = delete;
	X& operator=(const X&) = delete;
	X(X&&) = default;
	X& operator=(X&&) = default;

	static constexpr auto kTypeName = "x";
	template<typename S, typename V>void AcceptVisitor(S& self, V& v) {}
};

struct Modify : Visitor<> {
	ResultType operator()(A& a) {
		a.value += 1;
	}

	ResultType operator()(B& b) {}
	ResultType operator()(E& e) {
		e = E::kTwo;
	}
};

struct Check : Visitor<int> {
	ResultType operator()(const A& a) { return 1; }
	ResultType operator()(const B& b) { return 2; }
	ResultType operator()(const E& e) {	return 3; }
};

} // namespace

namespace std {

template<>
struct hash<A> {
	using argument_type = A;
	using result_type = size_t;

	result_type operator()(const argument_type& v) const { return v.value; }
};

template<>
struct hash<B> {
	using argument_type = B;
	using result_type = size_t;

	result_type operator()(const argument_type& v) const { return 0; }
};

template<>
struct hash<E> {
	using argument_type = E;
	using result_type = size_t;

	result_type operator()(const argument_type& v) const { return int(v.value); }
};

} // namespace std

TEST(VariantTest, CreateAndAssign) {
	Variant<A> v1;
	EXPECT_FALSE(v1.Is<A>());
	EXPECT_TRUE(v1.IsEmpty());

	v1 = A{};
	EXPECT_TRUE(v1.Is<A>());
	EXPECT_FALSE(v1.IsEmpty());

	Variant<A> v2(v1);
	EXPECT_TRUE(v2.Is<A>());

	Variant<A> v3(std::move(v2));
	EXPECT_TRUE(v3.Is<A>());

	v2.Clear();
	v1.Clear();

	v2 = v3;
	EXPECT_TRUE(v2.Is<A>());

	v1 = std::move(v2);
	EXPECT_TRUE(v1.Is<A>());

	A a1, a2;
	Variant<A> v4(std::move(a1));
	EXPECT_TRUE(v4.Is<A>());

	a2.value = 17;
	v4.Clear();
	EXPECT_FALSE(v4.Is<A>());

	v4 = std::move(a2);
	EXPECT_TRUE(v4.Is<A>());

	v1 = A{};
	v2.Clear();

	v1 = v2;
	EXPECT_TRUE(v1.IsEmpty());

	v1 = A{};
	v1 = std::move(v2);
	EXPECT_TRUE(v1.IsEmpty());

	v1 = A{};
	v2 = A{};
	v2.Get<A>().value = 33;

	v1 = v2;
	EXPECT_EQ(33, v1.Get<A>().value);

	v1 = A{};
	v1 = std::move(v2);
	EXPECT_EQ(33, v1.Get<A>().value);
}

TEST(VariantTest, Index) {
	using Vx = Variant<A, B, E>;
	Vx v1;

	EXPECT_EQ(Vx::Index(0), Vx::IndexOf<A>());
	EXPECT_EQ(Vx::Index(1), Vx::IndexOf<B>());
	EXPECT_EQ(Vx::Index(2), Vx::IndexOf<E>());

	EXPECT_EQ(Vx::Index(-1), v1.Which());

	v1 = A{};
	EXPECT_EQ(Vx::IndexOf<A>(), v1.Which());

	v1 = B{};
	EXPECT_EQ(Vx::IndexOf<B>(), v1.Which());

	v1.Clear();
	EXPECT_EQ(Vx::Index(-1), v1.Which());
}

TEST(VariantTest, GetAndEq) {
	using Vx = Variant<A, B, E>;

	Vx v1;
	const auto& v2 = v1;

	A a;
	a.value = 137;
	v1 = std::move(a);

	EXPECT_TRUE(v1.Is<A>());
	EXPECT_EQ(137, v1.Get<A>().value);
	EXPECT_EQ(137, v2.Get<A>().value);

	Vx v3;

	EXPECT_NE(v1, v3);
	EXPECT_EQ(Vx{}, v3);

	v3 = A{};
	EXPECT_NE(v1, v3);

	v3.Get<A>().value = 137;
	EXPECT_EQ(v1, v3);
}

TEST(VariantTest, Visitor) {
	using Vx = Variant<A, B, E>;

	Vx v1(A{});
	const auto& v2 = v1;

	EXPECT_FALSE(v1.IsEmpty());
	EXPECT_EQ(5, v1.Get<A>().value);

	v1.ApplyVisitor(Modify{});
	EXPECT_EQ(6, v1.Get<A>().value);
	EXPECT_EQ(1, v2.ApplyVisitor(Check{}));

	v1 = E{};
	v1.Get<E>().value = E::kOne;
	v1.ApplyVisitor(Modify{});
	EXPECT_EQ(E::kTwo, v1.Get<E>().value);
	EXPECT_EQ(3, v2.ApplyVisitor(Check{}));
}

TEST(VariantTest, Hash) {
	using Vx = Variant<A, B, int>;
	std::unordered_map<Vx, int> m;

	Vx v1(A{});
	Vx v2(A{});
	Vx v3(B{});

	v2.Get<A>().value = 37;

	m[Vx{}] = 7;
	m[v1] = 13;
	m[v2] = 21;

	EXPECT_NE(m.end(), m.find(Vx{}));
	EXPECT_NE(m.end(), m.find(v1));
	EXPECT_NE(m.end(), m.find(v2));
	EXPECT_EQ(m.end(), m.find(v3));

	EXPECT_EQ(7, m.find(Vx{})->second);
	EXPECT_EQ(13, m.find(v1)->second);
	EXPECT_EQ(21, m.find(v2)->second);
}

TEST(VariantTest, CopyNoCopy) {
	using Vx = Variant<A, X>;

	A a;
	X x;

	a.value = 13;
	Vx v1(a);
	a.value = 37;

	EXPECT_TRUE(v1.Is<A>());
	EXPECT_EQ(13, v1.Get<A>().value);

	v1 = std::move(a);
	EXPECT_EQ(37, v1.Get<A>().value);

	v1 = std::move(x);
	EXPECT_TRUE(v1.Is<X>());

	a.value = 7;
	v1 = a;
	EXPECT_TRUE(v1.Is<A>());
	EXPECT_EQ(7, v1.Get<A>().value);
}
