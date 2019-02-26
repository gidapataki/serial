#pragma once
#include <type_traits>


namespace serial {

template<typename T>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs,
	T*& root_ref)
{
	static_assert(std::is_base_of<ReferableBase, T>::value, "Invalid type");

	RefContainer result;
	BasicRef result_ref;

	auto ec = DeserializeObjects(root, reg, result, result_ref);
	if (ec != ErrorCode::kNone) {
		return ec;
	}

	if (!std::is_same<ReferableBase, T>::value && !result_ref.Is<T>()) {
		return ErrorCode::kInvalidRootType;
	}

	root_ref = static_cast<T*>(result_ref.Get());
	std::swap(result, refs);

	return ErrorCode::kNone;
}

template<typename... Ts>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs,
	TypedRef<Ts...>& root_ref)
{
	RefContainer result;
	BasicRef result_ref;

	auto ec = DeserializeObjects(root, reg, result, result_ref);
	if (ec != ErrorCode::kNone) {
		return ec;
	}

	if (!root_ref.Set(result_ref.Get())) {
		return ErrorCode::kInvalidRootType;
	}

	std::swap(result, refs);
	return ErrorCode::kNone;
}

} // namespace serial
