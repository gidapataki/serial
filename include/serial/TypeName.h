#pragma once
#include "serial/TypeTraits.h"

namespace serial {

template<typename T, typename = typename TypeTag<T>::Type>
struct TypeName;

template<typename T> struct TypeName<T, ObjectTag> { static constexpr const char* value = T::kTypeName; };
template<typename T> struct TypeName<T, EnumTag> { static constexpr const char* value = T::kTypeName; };
template<typename T> struct TypeName<T, ReferableTag> { static constexpr const char* value = T::kTypeName; };
template<typename T> struct TypeName<T, UserTag> { static constexpr const char* value = T::kTypeName; };

template<> struct TypeName<bool> { static constexpr auto value = "_bool_"; };
template<> struct TypeName<int32_t> { static constexpr auto value = "_i32_"; };
template<> struct TypeName<int64_t> { static constexpr auto value = "_i64_"; };
template<> struct TypeName<uint32_t> { static constexpr auto value = "_u32_"; };
template<> struct TypeName<uint64_t> { static constexpr auto value = "_u64_"; };
template<> struct TypeName<float> { static constexpr auto value = "_f32_"; };
template<> struct TypeName<double> { static constexpr auto value = "_f64_"; };
template<> struct TypeName<std::string> { static constexpr auto value = "_string_"; };

} // namespace serial
