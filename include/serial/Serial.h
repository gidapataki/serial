#pragma once
#include <vector>
#include <string>
#include "serial/SerialFwd.h"
#include "serial/Referable.h"
#include "serial/DocInfo.h"
#include "serial/Constants.h"
#include "jsoncpp/json.h"


namespace serial {

ErrorCode Serialize(
	Ref ref,
	const Header& header,
	const Registry& reg,
	Json::Value& value);

ErrorCode DeserializeHeader(
	const Json::Value& root,
	Header& header);

ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	std::vector<UniqueRef>& refs);

} // namespace serial
