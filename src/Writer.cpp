#include "serial/Writer.h"
#include "serial/ReferableBase.h"


namespace serial {

Writer::StateSentry::StateSentry(Writer* writer)
	: writer_(writer)
	, current_(writer->current_)
{}

Writer::StateSentry::~StateSentry() {
	writer_->current_ = current_;
}

Writer::Writer(const Registry& reg)
	: reg_(reg)
{}

Writer::Writer(const Registry& reg, noasserts_t)
	: Writer(reg)
{
	enable_asserts_ = false;
}

int Writer::Add(const ReferableBase* ref) {
	auto it = refids_.find(ref);
	if (it != refids_.end()) {
		return it->second;
	}
	auto id = next_refid_++;
	refids_[ref] = id;
	remaining_refs_.insert(ref);
	return id;
}

ErrorCode Writer::Write(
	const Header& header, ReferableBase* ref, Json::Value& output)
{
	root_ = Json::Value(Json::objectValue);


	StateSentry sentry(this);
	auto root_id = Add(ref);

	Current()[str::kDocType] = Json::Value(header.doctype);
	Current()[str::kDocVersion] = Json::Value(header.version);
	Current()[str::kRootId] = Json::Value(root_id);
	Select(str::kObjects) = Json::Value(Json::arrayValue);

	while (!remaining_refs_.empty()) {
		StateSentry sentry2(this);
		SelectNext();
		auto p = remaining_refs_.begin();
		auto ref = *p;
		remaining_refs_.erase(p);
		ref->Write(this);
		if (error_ != ErrorCode::kNone) {
			return error_;
		}
	}

	output = root_;
	return ErrorCode::kNone;
}

void Writer::VisitValue(const float& value, PrimitiveTag) {
	if (std::isnan(value)) {
		Current() = "nan";
	} else if (std::isinf(value)) {
		Current() = (value < 0 ? "-inf" : "inf");
	} else {
		Current() = Json::Value(value);
	}
}

void Writer::VisitValue(const double& value, PrimitiveTag) {
	if (std::isnan(value)) {
		Current() = "nan";
	} else if (std::isinf(value)) {
		Current() = (value < 0 ? "-inf" : "inf");
	} else {
		Current() = Json::Value(value);
	}
}

Json::Value& Writer::Select(const char* name) {
	current_ = &Current()[name];
	return Current();
}

Json::Value& Writer::SelectNext() {
	current_ = &Current().append({});
	return Current();
}

Json::Value& Writer::Current() {
	return *current_;
}

void Writer::SetError(ErrorCode error) {
	error_ = error;
}

} // namespace serial
