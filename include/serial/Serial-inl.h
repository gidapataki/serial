#pragma once
#include <type_traits>
#include "serial/Registry.h"


namespace serial {

template<typename T>
ErrorCode Serialize(
	T& obj,
	const Header& header,
	Json::Value& value)
{
	static_assert(std::is_base_of<ReferableBase, T>::value, "Invalid type");

	Registry reg;
	if (!reg.RegisterAll<T>()) {
		return ErrorCode::kInvalidSchema;
	}
	return Writer(reg).Write(header, &obj, value);
}

template<typename T>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	RefContainer& refs,
	T*& root_ref)
{
	static_assert(
		std::is_base_of<ReferableBase, T>::value &&
		!std::is_same<ReferableBase, T>::value, "Invalid type");

	RefContainer result;
	ReferableBase* result_ref = nullptr;

	Header h;
	Reader reader(root);
	auto ec = reader.ReadHeader(h);
	if (ec != ErrorCode::kNone) {
		return ec;
	}

	Registry reg;
	if (!reg.RegisterAll<T>()) {
		return ErrorCode::kInvalidSchema;
	}
	ec = reader.ReadObjects(reg, result, result_ref);
	if (ec != ErrorCode::kNone) {
		return ec;
	}

	if (result_ref->GetTypeId() != StaticTypeId<T>::Get()) {
		return ErrorCode::kInvalidRootType;
	}

	root_ref = static_cast<T*>(result_ref);
	std::swap(result, refs);

	return ErrorCode::kNone;
}

} // namespace serial
