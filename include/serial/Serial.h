#pragma once
#include <vector>
#include <string>
#include "serial/SerialFwd.h"
#include "serial/Referable.h"
#include "serial/Header.h"
#include "serial/Constants.h"
#include "serial/TypedRef.h"
#include "serial/BasicRef.h"
#include "jsoncpp/json.h"


namespace serial {

/**
 * Serialize a `BasicRef` to a `Json::Value`.
 * @value    Result of the serialization, only set on success.
 * @return   ErrorCode::kNone on success, specific errorcode otherwise.
 */
ErrorCode Serialize(
	BasicRef ref,
	const Header& header,
	const Registry& reg,
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
ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs,
	BasicRef& root_ref);

/**
 * Deserialize objects from a `Json::Value`.
 * @refs      Objects found during the deserialization, only set on success.
 * @root_ref  Root object, only set on success.
 * @return 	  ErrorCode::kNone on success, specific errorcode otherwise.
 */
template<typename... Ts>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs,
	TypedRef<Ts...>& root_ref);

/**
 * Deserialize objects from a `Json::Value`.
 * @refs      Objects found during the deserialization, only set on success.
 * @root_ref  Root object, only set on success.
 * @return 	  ErrorCode::kNone on success, specific errorcode otherwise.
 */
template<typename T>
ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs,
	T*& root_ref);

} // namespace serial

#include "serial/Serial-inl.h"
