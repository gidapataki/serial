#pragma once
#include "serial/Registry.h"

namespace serial {

template<typename... Ts, typename U>
struct Reader::ForEachVariantType<Variant<Ts...>, detail::Typelist<U>> {
	using V = Variant<Ts...>;

	static bool ReadIf(V& variant, TypeId id, Reader* reader) {
		using Info = VersionedTypeInfo<U>;
		using Type = typename Info::Type;

		if (StaticTypeId<Type>::Get() == id) {
			if (!reader->IsVersionInRange(Info::Begin(), Info::End())) {
				return false;
			}

			variant = Type{};
			reader->ReadVariant(variant.template Get<Type>());
			return true;
		}
		return false;
	}
};

template<typename... Ts, typename U, typename... Us>
struct Reader::ForEachVariantType<Variant<Ts...>, detail::Typelist<U, Us...>> {
	using V = Variant<Ts...>;
	static bool ReadIf(V& variant, TypeId id, Reader* reader) {
		return
			ForEachVariantType<V, detail::Typelist<U>>::ReadIf(variant, id, reader) ||
			ForEachVariantType<V, detail::Typelist<Us...>>::ReadIf(variant, id, reader);
	}
};

// Reader

template<typename T>
void Reader::VisitField(
	T& value, const char* name, BeginVersion v0, EndVersion v1)
{
	if (IsError()) {
		return;
	}

	if (!IsVersionInRange(v0, v1)) {
		return;
	}

	if (!Current().isMember(name)) {
		SetError(ErrorCode::kMissingObjectField);
		return;
	}

	++state_.processed;
	StateSentry sentry(this);
	Select(name);
	VisitValue(value);
}

template<typename T>
void Reader::ReadReferable(T& value) {
	auto input_count = Current().size();
	StateSentry sentry(this);
	state_.processed = 0;

	T::AcceptVisitor(value, *this);
	if (!IsError() && state_.processed < input_count) {
		SetError(ErrorCode::kUnexpectedObjectField);
	}
}

template<typename T>
void Reader::ReadVariant(T& value) {
	StateSentry sentry(this);
	assert(Current()[str::kVariantType] == TypeName<T>::value);
	Select(str::kVariantValue);
	VisitValue(value);
}

template<typename T>
void Reader::VisitValue(T& value) {
	typename TypeTag<T>::Type tag;
	VisitValue(value, tag);
}


template<typename T>
void Reader::VisitValue(T& value, ArrayTag) {
	if (!Current().isArray()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value.reserve(Current().size());

	for (auto& element : Current()) {
		if (IsError()) {
			return;
		}

		StateSentry sentry(this);
		Select(element);
		value.emplace_back();
		VisitValue(value.back());
	}
}

template<typename T>
void Reader::VisitValue(T& value, OptionalTag) {
	static_assert(!std::is_same<
		OptionalTag,
		typename TypeTag<typename T::value_type>::Type>::value,
		"Cannot nest Optional types");

	if (Current().isNull()) {
		value = boost::none;
	} else {
		StateSentry sentry(this);
		using ValueType = typename T::value_type;
		value = ValueType{};
		VisitValue(*value);
	}
}

template<typename T>
void Reader::VisitValue(T& value, ObjectTag) {
	if (!Current().isObject()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	auto input_count = Current().size();
	state_.processed = 0;
	T::AcceptVisitor(value, *this);
	if (!IsError() && state_.processed < input_count) {
		SetError(ErrorCode::kUnexpectedObjectField);
		return;
	}
}

template<typename T>
void Reader::VisitValue(T& value, UserTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	if (!value.FromString(Current().asString())) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}
}

template<typename T>
void Reader::VisitValue(T& value, EnumTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	if (!reg_->EnumFromString(Current().asString(), value)) {
		SetError(ErrorCode::kInvalidEnumValue);
		return;
	}
}

template<typename T>
void Reader::VisitValue(T& value, RefTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	auto refid = Current().asString();
	unresolved_refs_.emplace_back(&value, std::move(refid));
}

template<typename T>
void Reader::VisitValue(T& value, VariantTag) {
	if (!CheckVariant()) {
		return;
	}

	StateSentry sentry(this);
	auto type = Current()[str::kVariantType].asString();
	auto id = reg_->FindTypeId(type);

	if (id == kInvalidTypeId) {
		SetError(ErrorCode::kUnregisteredType);
		return;
	}

	auto success = ForEachVariantType<T, typename T::VersionedTypes>::ReadIf(value, id, this);
	if (!success) {
		SetError(ErrorCode::kInvalidVariantType);
	}
}

} // namespace serial
