#include <iostream> // todo - remove
#include "serial/Serial.h"
#include "serial/Writer.h"


namespace serial {

// Serialize

ErrorCode Serialize(
	BasicRef ref,
	const Header& header,
 	const Registry& reg,
 	Json::Value& result)
{
	return Writer(reg).Write(header, ref.Get(), result);
}

ErrorCode DeserializeHeader(
	const Json::Value& root,
	Header& header)
{
	return Reader(root).ReadHeader(header);
}

ErrorCode DeserializeObjects(
	const Json::Value& root,
	const Registry& reg,
	RefContainer& refs)
{
	Header h;
	Reader reader(root);
	auto ec = reader.ReadHeader(h);
	if (ec != ErrorCode::kNone) {
		return ec;
	}

	return reader.ReadObjects(reg, refs);
}

} // namespace serial
