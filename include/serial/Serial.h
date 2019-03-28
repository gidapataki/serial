#pragma once
#include <vector>
#include <string>
#include "serial/SerialFwd.h"
#include "serial/Referable.h"
#include "serial/Header.h"
#include "serial/Constants.h"
#include "serial/Ref.h"
#include "serial/Variant.h"
#include "jsoncpp/json.h"


namespace serial {

/**
 * Serialize an object to a `Json::Value`.
 * @value    Result of the serialization, only set on success.
 * @return   ErrorCode::kNone on success, specific errorcode otherwise.
 */
template<typename T>
ErrorCode Serialize(
	const T& obj,
	const Header& header,
	Json::Value& value);

/**
 * Deserialize a Header from a `Json::Value`.
 * @header    Result of the deserialization, only set on success.
 * @return 	  ErrorCode::kNone on success, specific errorcode otherwise.
 */
ErrorCode DeserializeHeader(
	const Json::Value& root,
	Header& header);

/**
 * Deserialize objects from a `Json::Value`.
 * @refs      Objects found during the deserialization, only set on success.
 * @root_ref  Root object, only set on success.
 * @return 	  ErrorCode::kNone on success, specific errorcode otherwise.
 */
template<typename T>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	RefContainer& refs,
	T*& root_ref);

} // namespace serial

#include "serial/Serial-inl.h"
