#include "serial/Serial.h"
#include "serial/Writer.h"


namespace serial {

noasserts_t noasserts;

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
	RefContainer& refs,
	BasicRef& root_ref)
{
	Header h;
	Reader reader(root);
	auto ec = reader.ReadHeader(h);
	if (ec != ErrorCode::kNone) {
		return ec;
	}

	RefContainer result;
	ReferableBase* result_ref = nullptr;

	ec = reader.ReadObjects(reg, result, result_ref);
	if (ec == ErrorCode::kNone) {
		std::swap(refs, result);
		root_ref = result_ref;
	}
	return ec;
}

} // namespace serial
