#pragma once
#include "serial/SerialFwd.h"
#include <string>
#include <vector>
#include <iosfwd>
#include <sstream>
#include <algorithm>


namespace serial {

namespace detail {

template<typename... Ts>
struct ReferableNames;

template<typename T>
struct ReferableNames<T> {
	static void Add(std::vector<std::string>& vec) {
		vec.emplace_back(T::GetReferableName());
	}
};

template<typename T, typename... Ts>
struct ReferableNames<T, Ts...> {
	static void Add(std::vector<std::string>& vec) {
		vec.emplace_back(T::GetReferableName());
		ReferableNames<Ts...>::Add(vec);
	}
};

template<typename T>
struct ReferableNamesOf;


template<typename... Ts>
struct ReferableNamesOf<TypedRef<Ts...>> {
	using type = ReferableNames<Ts...>;
};

} // namespace


class Blueprint {
	struct State {
		std::string prefix;
	} state_;

	class StateSentry {
	public:
		StateSentry(Blueprint* parent)
			: parent_(parent)
			, state_(parent->state_)
		{}

		~StateSentry() {
			parent_->state_ = state_;
		}

	private:
		State state_;
		Blueprint* parent_ = nullptr;
	};

public:
	template<typename T>
	void Add() {
		StateSentry sentry(this);
		state_.prefix = T::GetReferableName();
		T elem;
		T::AcceptVisitor(elem, *this);
	}

	void Dump(std::ostream& stream) {
		for (auto& x : output_) {
			stream << x.first << " : " << x.second << std::endl;
		}
	}


	template<typename T>
	void VisitField(const T& value, const char* name) {
		StateSentry sentry(this);
		state_.prefix += '.';
		state_.prefix += name;
		VisitValue(value);
	}

	template<typename T>
	void VisitValue(const T& value) {
		using Tag = typename TypeTag<T>::Type;
		Tag tag;
		VisitValue(value, tag);
	}

	template<typename T>
	void VisitValue(const T& value, PrimitiveTag) {
		VisitPrimitive(value);
	}

	void DeclareType(const std::string& name) {
		output_.emplace_back(state_.prefix, name);
	}

	void VisitPrimitive(const bool&) { DeclareType("bool"); }
	void VisitPrimitive(const int32_t&) { DeclareType("i32"); }
	void VisitPrimitive(const int64_t&) { DeclareType("i64"); }
	void VisitPrimitive(const uint32_t&) { DeclareType("u32"); }
	void VisitPrimitive(const uint64_t&) { DeclareType("u64"); }
	void VisitPrimitive(const float&) { DeclareType("float"); }
	void VisitPrimitive(const double&) { DeclareType("double"); }
	void VisitPrimitive(const std::string&) { DeclareType("string"); }

	template<typename T>
	void VisitValue(const Array<T>& value, ArrayTag) {
		StateSentry sentry(this);

		state_.prefix += "[]";
		T elem;
		VisitValue(elem);
	}

	template<typename T>
	void VisitValue(const Optional<T>& value, OptionalTag) {
		StateSentry sentry(this);

		state_.prefix += "?";
		T elem;
		VisitValue(elem);
	}

	template<typename T>
	void VisitValue(const T& value, ObjectTag) {
		StateSentry sentry(this);
		T::AcceptVisitor(value, *this);
	}

	template<typename T>
	void VisitValue(const T& value, EnumTag) {
		auto id = StaticTypeId<T>::Get();
		int index = next_enum_;

		if (enums_.count(id) > 0) {
			index = enums_[id];
		} else {
			enums_[id] = next_enum_++;
		}

		DeclareType("enum-" + std::to_string(index));
	}

	template<typename T>
	void VisitValue(const T& value, TypedRefTag) {
		using R = typename detail::ReferableNamesOf<T>::type;
		std::vector<std::string> vec;
		R::Add(vec);
		std::stringstream ss;
		std::sort(vec.begin(), vec.end());
		ss << "ref<";
		int index = 0;
		for (auto& x : vec) {
			if (index > 0) {
				ss << ", ";
			}
			ss << x;
			++index;
		}
		ss << ">";

		DeclareType(ss.str());
	}

	template<typename T>
	void VisitValue(const T& value, UserTag) {
		std::stringstream ss;
		ss << "user<" << T::kPrimitiveName << ">";
		DeclareType(ss.str());
	}

private:
	std::vector<std::pair<std::string, std::string>> output_;
	std::unordered_map<TypeId, int> enums_;
	int next_enum_ = 0;
};

} // namespace serial
