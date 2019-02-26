#pragma once
#include <type_traits>


namespace serial {

template<typename T>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs,
	T*& typed_root)
{
	static_assert(std::is_base_of<ReferableBase, T>::value, "Invalid type");
	auto ec = DeserializeObjects(root, reg, refs);
	if (ec != ErrorCode::kNone) {
		return ec;
	}
	if (!std::is_same<ReferableBase, T>::value && !refs[0]->HasType<T>()) {
		return ErrorCode::kInvalidRootType;
	}
	typed_root = static_cast<T*>(refs[0].get());
	return ErrorCode::kNone;
}

} // namespace serial
