#include "serial/Reader.h"
#include "serial/ReferableBase.h"
#include "serial/Registry.h"
#include "serial/Ref.h"
#include <limits>

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

	if (!Current().isMember(str::kDocType) ||
		!Current().isMember(str::kDocVersion) ||
		!Current().isMember(str::kRootId) ||
		!Current().isMember(str::kObjects))
	{
		return ErrorCode::kMissingHeaderField;
	}

	if (!Current()[str::kDocType].isString() ||
		!Current()[str::kDocVersion].isInt() ||
		!Current()[str::kRootId].isString() ||
		!Current()[str::kObjects].isArray())
	{
		return ErrorCode::kInvalidHeader;
	}

	if (Current().size() > 4) {
		return ErrorCode::kUnexpectedHeaderField;
	}

	header.doctype = Current()[str::kDocType].asString();
	header.version = Current()[str::kDocVersion].asInt();
	return ErrorCode::kNone;
}

ErrorCode Reader::ReadObjects(
	const Registry& reg, RefContainer& refs, ReferableBase*& root)
{
	if (!Current().isObject()) {
		return ErrorCode::kInvalidDocument;
	}

	SetError(ErrorCode::kNone);
	ReadObjectsInternal(reg);
	if (IsError()) {
		return error_;
	}

	ResolveRefs();
	if (IsError()) {
		return error_;
	}

	ExtractRefs(refs, root);
	if (IsError()) {
		return error_;
	}
	return ErrorCode::kNone;
}

void Reader::ReadObjectsInternal(const Registry& reg) {
	StateSentry sentry(this);
	auto& root_value = Current()[str::kRootId];
	if (!root_value.isString()) {
		SetError(ErrorCode::kInvalidHeader);
		return;
	}

	root_id_ = root_value.asString();

	Select(str::kObjects);
	if (!Current().isArray() ||
		Current().size() == 0) {
		SetError(ErrorCode::kMissingRootObject);
		return;
	}

	for (const auto& value : Current()) {
		StateSentry sentry2(this);
		Select(value);
		ReadObjectInternal(reg);
		if (IsError()) {
			return;
		}
	}
}

void Reader::ReadObjectInternal(const Registry& reg) {
	StateSentry sentry(this);

	if (!Current().isMember(str::kObjectFields) ||
		!Current().isMember(str::kObjectType) ||
		!Current().isMember(str::kObjectId))
	{
		SetError(ErrorCode::kMissingHeaderField);
		return;
	}

	if (!Current()[str::kObjectFields].isObject() ||
		!Current()[str::kObjectType].isString() ||
		!Current()[str::kObjectId].isString())
	{
		SetError(ErrorCode::kInvalidObjectHeader);
		return;
	}

	if (Current().size() > 3) {
		SetError(ErrorCode::kUnexpectedHeaderField);
		return;
	}

	auto type = Current()[str::kObjectType].asString();
	auto id = Current()[str::kObjectId].asString();

	if (objects_.find(id) != objects_.end()) {
		SetError(ErrorCode::kDuplicateObjectId);
		return;
	}

	auto obj = reg.CreateReferable(type);
	if (!obj) {
		SetError(ErrorCode::kUnregisteredType);
		return;
	}

	Select(str::kObjectFields);
	auto p = obj.get();
	objects_[id] = std::move(obj);

	reg_ = &reg;
	p->Read(this);
	reg_ = nullptr;
}

void Reader::ResolveRefs() {
	for (auto& instance : unresolved_refs_) {
		auto refptr = instance.first;
		auto refid = instance.second;
		auto it = objects_.find(refid);
		if (it == objects_.end()) {
			SetError(ErrorCode::kUnresolvableReference);
			return;
		}

		if (!refptr->Set(it->second.get())) {
			SetError(ErrorCode::kInvalidReferenceType);
			return;
		}
	}
}

void Reader::ExtractRefs(RefContainer& refs, ReferableBase*& root) {
	RefContainer result;
	auto it = objects_.find(root_id_);

	if (it == objects_.end()) {
		SetError(ErrorCode::kMissingRootObject);
		return;
	}

	auto root_ref = it->second.get();
	for (auto& obj : objects_) {
		result.push_back(std::move(obj.second));
	}

	root = root_ref;
	std::swap(result, refs);
}

void Reader::VisitValue(bool& value, PrimitiveTag) {
	if (!Current().isBool()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asBool();
}

void Reader::VisitValue(int& value, PrimitiveTag) {
	if (!Current().isInt()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asInt();
}

void Reader::VisitValue(int64_t& value, PrimitiveTag) {
	if (!Current().isInt64()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asInt64();
}

void Reader::VisitValue(unsigned& value, PrimitiveTag) {
	if (!Current().isUInt()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asUInt();
}

void Reader::VisitValue(uint64_t& value, PrimitiveTag) {
	if (!Current().isUInt64()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asUInt64();
}

void Reader::VisitValue(float& value, PrimitiveTag) {
	if (Current().isString()) {
		auto v = Current().asString();
		if (v == "nan") {
			value = std::numeric_limits<float>::quiet_NaN();
		} else if (v == "inf") {
			value = std::numeric_limits<float>::infinity();
		} else if (v == "-inf") {
			value = -std::numeric_limits<float>::infinity();
		} else {
			SetError(ErrorCode::kInvalidObjectField);
		}
		return;
	}

	if (!Current().isDouble()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	float v = static_cast<float>(Current().asDouble());
	if (std::isinf(v) || std::isnan(v)) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = v;
}

void Reader::VisitValue(double& value, PrimitiveTag) {
	if (Current().isString()) {
		auto v = Current().asString();
		if (v == "nan") {
			value = std::numeric_limits<double>::quiet_NaN();
		} else if (v == "inf") {
			value = std::numeric_limits<double>::infinity();
		} else if (v == "-inf") {
			value = -std::numeric_limits<double>::infinity();
		} else {
			SetError(ErrorCode::kInvalidObjectField);
		}
		return;
	}

	if (!Current().isDouble()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	double v = Current().asDouble();
	if (std::isinf(v) || std::isnan(v)) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asDouble();
}

void Reader::VisitValue(std::string& value, PrimitiveTag) {
	if (!Current().isString()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value = Current().asString();
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
