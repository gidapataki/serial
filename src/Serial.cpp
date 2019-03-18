#include "serial/Serial.h"
#include "serial/Writer.h"


namespace serial {

noasserts_t noasserts;

// Serialize

ErrorCode DeserializeHeader(
	const Json::Value& root,
	Header& header)
{
	return Reader(root).ReadHeader(header);
}

} // namespace serial
