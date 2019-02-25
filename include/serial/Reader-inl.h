#pragma once


namespace serial {

template<typename T>
void Reader::ReadReferable(T& value) {
	if (!Current().isObject()) {
		SetError(ErrorCode::kInvalidObjectHeader);
		return;
	}

	auto input_count = Current().size();
	StateSentry sentry(this);
	state_.processed = 0;

	T::AcceptVisitor(value, *this);
	if (state_.processed < input_count) {
		SetError(ErrorCode::kUnexpectedObjectField);
	}
}

template<typename T>
void Reader::VisitField(T& value, const char* name) {
	if (IsError()) {
		return;
	}

	if (!Current().isMember(name)) {
		SetError(ErrorCode::kMissingObjectField);
		return;
	}

	++state_.processed;
	StateSentry sentry(this);
	Select(name);
	VisitValue(value);
}

template<typename T>
void Reader::VisitValue(T& value) {
	typename TypeTag<T>::Type tag;
	VisitValue(value, tag);
}


template<typename T>
void Reader::VisitValue(T& value, ArrayTag) {
	if (!Current().isArray()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	value.reserve(Current().size());

	for (auto& element : Current()) {
		if (IsError()) {
			return;
		}

		StateSentry sentry(this);
		Select(element);
		value.emplace_back();
		VisitValue(value.back());
	}
}

template<typename T>
void Reader::VisitValue(T& value, ObjectTag) {
	if (!Current().isObject()) {
		SetError(ErrorCode::kInvalidObjectField);
		return;
	}

	auto input_count = Current().size();
	state_.processed = 0;
	T::AcceptVisitor(value, *this);
	if (state_.processed < input_count) {
		SetError(ErrorCode::kUnexpectedObjectField);
	}
}

} // namespace serial
