#pragma once
#include <vector>
#include <string>
#include "serial/SerialFwd.h"
#include "serial/Referable.h"
#include "serial/DocInfo.h"
#include "serial/Constants.h"
#include "serial/TypedRef.h"
#include "serial/BasicRef.h"
#include "jsoncpp/json.h"


namespace serial {

ErrorCode Serialize(
	BasicRef ref,
	const Header& header,
	const Registry& reg,
	Json::Value& value);

ErrorCode DeserializeHeader(
	const Json::Value& root,
	Header& header);

ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs);

template<typename T>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs,
	T*& typed_root);

} // namespace serial

#include "serial/Serial-inl.h"
