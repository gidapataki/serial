#include "serial/Reader.h"
#include "serial/ReferableBase.h"
#include "serial/Registry.h"


namespace serial {

Reader::StateSentry::StateSentry(Reader* reader)
	: reader_(reader)
	, state_(reader->state_)
{}

Reader::StateSentry::~StateSentry() {
	reader_->state_ = state_;
}

Reader::Reader(const Json::Value& root)
	: root_(std::move(root))
{
	state_.current = &root_;
}

ErrorCode Reader::ReadHeader(Header& header) {
	if (!Current().isObject()) {
		return ErrorCode::kInvalidDocument;
	}

	if (!Current().isMember(str::kDoctype) ||
		!Current().isMember(str::kVersion) ||
		!Current().isMember(str::kObjects))
	{
		return ErrorCode::kInvalidHeader;
	}

	if (!Current()[str::kDoctype].isString() ||
		!Current()[str::kVersion].isInt() ||
		!Current()[str::kObjects].isArray())
	{
		return ErrorCode::kInvalidHeader;
	}

	if (Current().size() > 3) {
		return ErrorCode::kUnexpectedHeaderField;
	}

	header.doctype = Current()[str::kDoctype].asString();
	header.version = Current()[str::kVersion].asInt();
	return ErrorCode::kNone;
}

ErrorCode Reader::ReadObjects(const Registry& reg, std::vector<UniqueRef>& refs) {
	SetError(ErrorCode::kNone);
	ReadObjectsInternal(reg);
	if (IsError()) {
		return error_;
	}

	if (!ResolveRefs()) {
		return ErrorCode::kUnresolvableReference;
	}

	ExtractRefs(refs);
	return ErrorCode::kNone;
}

void Reader::ReadObjectsInternal(const Registry& reg) {
	StateSentry sentry(this);
	Select(str::kObjects);
	if (!Current().isArray() ||
		Current().size() == 0) {
		SetError(ErrorCode::kMissingRootObject);
		return;
	}

	int index = -1;
	for (const auto& value : Current()) {
		++index;
		StateSentry sentry2(this);
		Select(value);
		ReadObjectInternal(index, reg);
		if (IsError()) {
			return;
		}
	}
}

void Reader::ReadObjectInternal(int index, const Registry& reg) {
	StateSentry sentry(this);

	if (!Current().isMember(str::kFields) ||
		!Current().isMember(str::kType) ||
		!Current().isMember(str::kId))
	{
		SetError(ErrorCode::kInvalidObjectHeader);
		return;
	}

	if (!Current()[str::kFields].isObject() ||
		!Current()[str::kType].isString() ||
		!Current()[str::kId].isInt())
	{
		SetError(ErrorCode::kInvalidObjectHeader);
		return;
	}

	if (Current().size() > 3) {
		SetError(ErrorCode::kUnexpectedHeaderField);
		return;
	}

	auto type = Current()[str::kType].asString();
	auto id = Current()[str::kId].asInt();

	if (objects_.find(id) != objects_.end()) {
		SetError(ErrorCode::kDuplicateObjectId);
		return;
	}

	auto obj = reg.Create(type);
	if (!obj) {
		SetError(ErrorCode::kUnregisteredType);
		return;
	}

	Select(str::kFields);
	auto p = obj.get();
	objects_[id] = std::move(obj);

	if (index == 0) {
		root_id_ = id;
	}

	reg_ = &reg;
	p->Read(this);
	reg_ = nullptr;
}

bool Reader::ResolveRefs() {
	for (auto& instance : unresolved_refs_) {
		auto refptr = instance.first;
		auto refid = instance.second;
		auto it = objects_.find(refid);
		if (it == objects_.end()) {
			return false;
		}

		*refptr = it->second.get();
	}
	return true;
}

void Reader::ExtractRefs(std::vector<UniqueRef>& refs) {
	std::vector<UniqueRef> result;

	auto root = objects_.find(root_id_);
	assert(root != objects_.end() && "Missing root object");

	result.push_back(std::move(root->second));
	objects_.erase(root);

	for (auto& obj : objects_) {
		result.push_back(std::move(obj.second));
	}

	std::swap(result, refs);
}

void Reader::VisitValue(int& value, PrimitiveTag) {
	if (!Current().isInt()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asInt();
}

void Reader::VisitValue(std::string& value, PrimitiveTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asString();
}

void Reader::VisitValue(Ref& value, RefTag) {
	if (!Current().isInt()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	auto refid = Current().asInt();
	unresolved_refs_.emplace_back(&value, refid);
}

void Reader::SetError(ErrorCode error) {
	error_ = error;
}

bool Reader::IsError() const {
	return error_ != ErrorCode::kNone;
}

const Json::Value& Reader::Current() {
	return *state_.current;
}

const Json::Value& Reader::Select(const char* name) {
	state_.current = &Current()[name];
	return Current();
}

const Json::Value& Reader::Select(const Json::Value& value) {
	state_.current = &value;
	return Current();
}

} // namespace serial
