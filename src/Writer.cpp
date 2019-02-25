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


void Writer::Add(Ref ref) {
	if (refids_.count(ref) == 0) {
		remaining_refs_.insert(ref);
		refids_[ref] = next_refid_++;
	}
}

ErrorCode Writer::Write(const Header& header, Ref ref, Json::Value& output) {
	root_ = Json::Value(Json::objectValue);

	StateSentry sentry(this);
	Current()[str::kDoctype] = Json::Value(header.doctype);
	Current()[str::kVersion] = Json::Value(header.version);
	Select(str::kObjects) = Json::Value(Json::arrayValue);

	Add(ref);
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


} // namespace serial
