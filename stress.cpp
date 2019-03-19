#include "serial/Serial.h"

using namespace serial;



template<int N>
struct A : Referable<A<N>> {
	int zero = N;

	static constexpr auto kReferableName = "a";

	template<typename Self, typename Visitor>
	static void AcceptVisitor(Self& self, Visitor& v) {
		v.VisitField(self.zero, "zero");
	}
};


template<int N>
void Touch(Registry& reg) {
	using T = A<N>;
	T t;
	reg.Register<T>();
}


int main() {
	Header h;
	Registry reg;
	int u = 12;

#if 1
	Touch<0>(reg);
	Touch<1>(reg);
	Touch<2>(reg);
	Touch<3>(reg);
	Touch<4>(reg);
	Touch<5>(reg);
	Touch<6>(reg);
	Touch<7>(reg);
	Touch<8>(reg);
	Touch<9>(reg);
#endif
#if 1
	Touch<10>(reg);
	Touch<11>(reg);
	Touch<12>(reg);
	Touch<13>(reg);
	Touch<14>(reg);
	Touch<15>(reg);
	Touch<16>(reg);
	Touch<17>(reg);
	Touch<18>(reg);
	Touch<19>(reg);
	Touch<20>(reg);
	Touch<21>(reg);
	Touch<22>(reg);
	Touch<23>(reg);
	Touch<24>(reg);
	Touch<25>(reg);
	Touch<26>(reg);
	Touch<27>(reg);
	Touch<28>(reg);
	Touch<29>(reg);
#endif
#if 1
	Touch<30>(reg);
	Touch<31>(reg);
	Touch<32>(reg);
	Touch<33>(reg);
	Touch<34>(reg);
	Touch<35>(reg);
	Touch<36>(reg);
	Touch<37>(reg);
	Touch<38>(reg);
	Touch<39>(reg);
	Touch<40>(reg);
	Touch<41>(reg);
	Touch<42>(reg);
	Touch<43>(reg);
	Touch<44>(reg);
	Touch<45>(reg);
	Touch<46>(reg);
	Touch<47>(reg);
	Touch<48>(reg);
	Touch<49>(reg);
	Touch<50>(reg);
	Touch<51>(reg);
	Touch<52>(reg);
	Touch<53>(reg);
	Touch<54>(reg);
	Touch<55>(reg);
	Touch<56>(reg);
	Touch<57>(reg);
	Touch<58>(reg);
	Touch<59>(reg);
#endif
#if 0
	Touch<60>(reg);
	Touch<61>(reg);
	Touch<62>(reg);
	Touch<63>(reg);
	Touch<64>(reg);
	Touch<65>(reg);
	Touch<66>(reg);
	Touch<67>(reg);
	Touch<68>(reg);
	Touch<69>(reg);
	Touch<70>(reg);
	Touch<71>(reg);
	Touch<72>(reg);
	Touch<73>(reg);
	Touch<74>(reg);
	Touch<75>(reg);
	Touch<76>(reg);
	Touch<77>(reg);
	Touch<78>(reg);
	Touch<79>(reg);
	Touch<80>(reg);
	Touch<81>(reg);
	Touch<82>(reg);
	Touch<83>(reg);
	Touch<84>(reg);
	Touch<85>(reg);
	Touch<86>(reg);
	Touch<87>(reg);
	Touch<88>(reg);
	Touch<89>(reg);
	Touch<90>(reg);
	Touch<91>(reg);
	Touch<92>(reg);
	Touch<93>(reg);
	Touch<94>(reg);
	Touch<95>(reg);
	Touch<96>(reg);
	Touch<97>(reg);
	Touch<98>(reg);
	Touch<99>(reg);
#endif

	A<1> a;
	Json::Value	root;
	auto ec = Serialize(a, h, reg, root);

	return 0;
}