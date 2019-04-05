#include "gtest/gtest.h"
#include "serial/Blueprint.h"
#include "serial/Serial.h"
#include <sstream>


using namespace serial;

namespace {

using Version1 = Version<1>;
using Version2 = Version<2>;

struct A : Referable<A> {
	static constexpr auto kTypeName = "A";
	int x;
	std::string s;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.x, "x");
		v.VisitField(self.s, "s");
	}
};

struct B {
	static constexpr auto kTypeName = "B";
	double z;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.z, "z");
	}
};

struct U : UserPrimitive {
	static constexpr auto kTypeName = "U";

	bool FromString(const std::string&) { return true; }
	bool ToString(std::string&) const { return true; }
};

struct E : Enum {
	enum Value : int {
		kRed,
		kGreen,
		kBlue,
	} value = {};

	E() = default;
	E(Value v) : value(v) {}

	static constexpr auto kTypeName = "E";

	template<typename V>
	static void AcceptVisitor(V& v) {
		v.VisitEnumValue(kRed, "red");
		v.VisitEnumValue(kGreen, "green");
		v.VisitEnumValue(kBlue, "blue", Version1(), Version2());
	}
};


struct C : Referable<C> {
	static constexpr auto kTypeName = "C";

	Ref<A, C(Version1, Version2)> a;
	B b;
	Variant<int, B(Version1, Version2)> v;
	Optional<int> opt;
	Array<int> arr;
	Array<Optional<int>> arr_opt;
	Array<Array<int>> mat;
	E e;
	U u;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.b, "b");
		v.VisitField(self.a, "a");
		v.VisitField(self.v, "v");
		v.VisitField(self.e, "e");
		v.VisitField(self.u, "u");
		v.VisitField(self.opt, "opt");
		v.VisitField(self.arr, "arr");
		v.VisitField(self.arr_opt, "arr_opt");
		v.VisitField(self.mat, "mat", Version1(), Version2());
	}
};


} // namespace

TEST(BlueprintTest, Ctor) {
	Blueprint bp;

	EXPECT_EQ(0, bp.Size());
	EXPECT_EQ(std::string{}, ToString(bp));

	Blueprint bp2;
	EXPECT_EQ(std::string{}, Diff(bp, bp2));
}

TEST(BlueprintTest, AddLine) {
	Blueprint bp;

	EXPECT_FALSE(bp.AddLine("x\ny"));
	EXPECT_EQ(0, bp.Size());

	EXPECT_TRUE(bp.AddLine("x y"));
	EXPECT_EQ(1, bp.Size());

	EXPECT_TRUE(bp.AddLine("x y"));
	EXPECT_EQ(1, bp.Size());

	EXPECT_TRUE(bp.AddLine(" x y "));
	EXPECT_EQ(1, bp.Size());

	EXPECT_TRUE(bp.AddLine(" x z"));
	EXPECT_EQ(2, bp.Size());

	EXPECT_TRUE(bp.AddLine("x z"));
	EXPECT_EQ(2, bp.Size());
}

TEST(BlueprintTest, FromString) {
	Blueprint bp;

	bp = Blueprint::FromString("");
	EXPECT_EQ(0, bp.Size());

	bp = Blueprint::FromString("a\nb");
	EXPECT_EQ(2, bp.Size());

	bp = Blueprint::FromString("  ");
	EXPECT_EQ(0, bp.Size());

	bp = Blueprint::FromString("x\n\n\nc\n");
	EXPECT_EQ(2, bp.Size());
}

TEST(BlueprintTest, ToString) {
	Blueprint bp1;
	Blueprint bp2;

	bp2.AddLine("x");
	EXPECT_EQ(std::string{""}, ToString(bp1));
	EXPECT_EQ(std::string{"x\n"}, ToString(bp2));

	bp1.AddLine("y");
	EXPECT_EQ(std::string{"y\n"}, ToString(bp1));
	EXPECT_EQ(std::string{"x\n"}, ToString(bp2));

	bp1.AddLine("x");
	EXPECT_EQ(std::string{"x\ny\n"}, ToString(bp1));
	EXPECT_EQ(std::string{"x\n"}, ToString(bp2));

	bp2.AddLine("y");
	EXPECT_EQ(std::string{"x\ny\n"}, ToString(bp1));
	EXPECT_EQ(std::string{"x\ny\n"}, ToString(bp2));
}

TEST(BlueprintTest, Diff) {
	Blueprint bp1;
	Blueprint bp2;

	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp1, bp2));
	EXPECT_NE(std::string{"hello"}, Diff(bp1, bp2));

	bp2.AddLine("x");
	EXPECT_EQ(std::string{"+ x\n"}, Diff(bp1, bp2));
	EXPECT_NE(Blueprint::kNoDiff, Diff(bp1, bp2));

	bp1.AddLine("y");
	EXPECT_EQ(std::string{"- y\n+ x\n"}, Diff(bp1, bp2));
	EXPECT_NE(Blueprint::kNoDiff, Diff(bp1, bp2));

	bp1.AddLine("x");
	EXPECT_EQ(std::string{"- y\n"}, Diff(bp1, bp2));
	EXPECT_NE(Blueprint::kNoDiff, Diff(bp1, bp2));

	bp2.AddLine("y");
	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp1, bp2));
}

TEST(BlueprintTest, Stream) {
	Blueprint::Delta d{"x\ny\nz"};
	std::stringstream ss;
	ss << d;
	EXPECT_EQ(std::string{"x\ny\nz"}, ss.str());
}

TEST(BlueprintTest, Union) {
	Blueprint bp1;
	Blueprint bp2;
	Blueprint bp3;

	bp1 = Blueprint::FromString("x\ny");
	bp2 = Blueprint::FromString("y\nz");
	bp3 = Union({bp1, bp2});

	EXPECT_EQ(std::string{"x\ny\nz\n"}, ToString(bp3));
}

TEST(BlueprintTest, FromType) {
	Blueprint bp;
	auto bp_a = Blueprint::FromString(R"(
		A :: referable
		A.x $ _i32_
		A.s $ _string_
	)");

	auto bp_b = Blueprint::FromString(R"(
		B :: object
		B.z $ _f64_
	)");

	auto bp_c0 = Blueprint::FromString(R"(
		C :: referable
		C.b.z $ _f64_
		C.a ref A
		C.e enum E
		C.u user U
		C.v variant _i32_
		C.opt? $ _i32_
		C.arr[] $ _i32_
		C.arr_opt[]? $ _i32_
	)");

	auto bp_c1 = Blueprint::FromString(R"(
		C.a ref C
		C.v variant B
		C.mat[][] $ _i32_
	)");

	auto bp_u = Blueprint::FromString("U :: usertype");

	auto bp_e0 = Blueprint::FromString(R"(
		E :: enum
		E option red
		E option green
	)");

	auto bp_e1 = Blueprint::FromString(R"(
		E option blue
	)");

	auto bp_cx0 = Union({bp_a, bp_c0, bp_u, bp_e0});
	auto bp_cx1 = Union({bp_cx0, bp_b, bp_c1, bp_e1});

	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp_a, Blueprint::FromType<A>()));
	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp_b, Blueprint::FromType<B>()));
	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp_u, Blueprint::FromType<U>()));
	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp_e0, Blueprint::FromType<E>()));
	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp_cx0, Blueprint::FromType<C>()));
	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp_cx1, Blueprint::FromType<C>(1)));
	EXPECT_EQ(Blueprint::kNoDiff, Diff(bp_cx0, Blueprint::FromType<C>(2)));
}
