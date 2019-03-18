#include <iostream>
#include <vector>
#include "serial/Serial.h"

using namespace serial;


void Dump(const Json::Value& root) {
	Json::StreamWriterBuilder builder;
	builder.settings_["indentation"] = "  ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &std::cout);
	std::cout << std::endl;
}



struct A : Referable<A> {
	int index = -1;
	Optional<TypedRef<A>> left;
	Optional<TypedRef<A>> right;

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.index, "i");
		v.VisitField(self.left, "l");
		v.VisitField(self.right, "r");
	}
};



int main() {
	Registry reg;
	reg.Register<A>("a");
	Json::Value root;
	Header h;

	int size = 30;
	int index = 0;
	int offset = 7;

	std::vector<A> xs(size);
	int next = 0;

	for (int i = 0; i < size; ++i) {
		xs[i].index = i;
		auto p = (next = (next + offset) % size);
		auto q = (next = (next + offset) % size);

		xs[i].left = &xs[p];
		xs[i].right = &xs[q];

		std::cerr << i << " -> " << p << " | " << q << std::endl;
	}

	auto ec = Serialize(&xs[0], h, reg, root);
	if (ec != ErrorCode::kNone) {
		return 1;
	}

	Dump(root);

	return 0;
}