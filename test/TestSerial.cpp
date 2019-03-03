#include "gtest/gtest.h"
#include "serial/BasicRef.h"
#include "serial/Referable.h"
#include "serial/Serial.h"


using namespace serial;

namespace {

enum class Color {
	kRed,
	kOrange,
	kYellow,
};

struct A : Referable<A> {
	int value;
	std::string name;

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.value, "value");
		v.VisitField(self.name, "name");
	}
};

struct B : Referable<B> {
	Color color = {};

	template<typename S, typename V>
	static void AcceptVisitor(S& self, V& v) {
		v.VisitField(self.color, "color");
	}
};

} // namespace

TEST(SerialTest, Serialize) {
	Json::Value root = 1;
	Registry reg(noasserts);
	Header h;
	B b;

	reg.Register<B>("b");
	EXPECT_EQ(ErrorCode::kUnregisteredEnum, Serialize(&b, h, reg, root));
	EXPECT_TRUE(root.isInt());
	EXPECT_EQ(1, root.asInt());

	reg.RegisterEnum<Color>({{Color::kRed, "red"}});
	EXPECT_EQ(ErrorCode::kNone, Serialize(&b, h, reg, root));
}
