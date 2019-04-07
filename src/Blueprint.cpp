#include <sstream>
#include <algorithm>
#include <iterator>
#include <cctype>
#include "serial/Blueprint.h"


namespace serial {

namespace {

void TrimLeft(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

void TrimRight(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void Trim(std::string &s) {
    TrimLeft(s);
    TrimRight(s);
}

} // namespace


// Blueprint

const Blueprint::Delta Blueprint::kNoDiff;

Blueprint::Delta::Delta(const std::string& value)
	: value(value)
{}

Blueprint Blueprint::FromString(const std::string& str) {
	Blueprint bp;
	std::stringstream ss(str);
	std::string line;
	while (std::getline(ss, line, '\n')) {
		bp.AddLine(line);
	}
	return bp;
}

bool Blueprint::AddLine(std::string line) {
	Trim(line);

	if (line.empty()) {
		return false;
	}

	if (line.find('\n') != std::string::npos) {
		return false;
	}

	lines_.insert(line);
	return true;
}

std::size_t Blueprint::Size() const {
	return lines_.size();
}

std::string ToString(const Blueprint& bp) {
	std::stringstream ss;
	ss << bp;
	return ss.str();
}

Blueprint Union(std::initializer_list<Blueprint> bps) {
	Blueprint result;
	for (const auto& bp : bps) {
		for (auto& x : bp.lines_) {
			result.lines_.insert(x);
		}
	}
	return result;
}

Blueprint::Delta Diff(const Blueprint& lhs, const Blueprint& rhs) {
	std::stringstream ss;
	std::vector<std::string> diff;

	if (lhs.lines_ == rhs.lines_) {
		return {};
	}

	std::set_difference(
		lhs.lines_.begin(), lhs.lines_.end(),
		rhs.lines_.begin(), rhs.lines_.end(),
		std::inserter(diff, diff.begin()));

	for (auto& x : diff) {
		ss << "- ";
		ss << x;
		ss << std::endl;
	}

	diff.clear();
	std::set_difference(
		rhs.lines_.begin(), rhs.lines_.end(),
		lhs.lines_.begin(), lhs.lines_.end(),
		std::inserter(diff, diff.begin()));

	for (auto& x : diff) {
		ss << "+ ";
		ss << x;
		ss << std::endl;
	}

	Blueprint::Delta delta;
	delta.value = ss.str();
	return ss.str();
}

std::ostream& operator<<(std::ostream& stream, const Blueprint& bp) {
	for (auto& l : bp.lines_) {
		stream << l << std::endl;
	}
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const Blueprint::Delta& delta) {
	return stream << delta.value;
}

bool operator==(const Blueprint::Delta& lhs, const Blueprint::Delta& rhs) {
	return lhs.value == rhs.value;
}

bool operator!=(const Blueprint::Delta& lhs, const Blueprint::Delta& rhs) {
	return lhs.value != rhs.value;
}


// BlueprintWriter::StateSentry

BlueprintWriter::StateSentry::StateSentry(BlueprintWriter* parent)
	: parent_(parent)
	, state_(parent->state_)
{}

BlueprintWriter::StateSentry::~StateSentry() {
	parent_->state_ = state_;
}


// BlueprintWriter

BlueprintWriter::BlueprintWriter(Blueprint& bp, int version)
	: blueprint_(bp)
	, version_(version)
{}

void BlueprintWriter::Push(const char* str) {
	state_.prefix += str;
}

void BlueprintWriter::PushQualifier(const char* str) {
	state_.prefix += ' ';
	state_.prefix += str;
}

void BlueprintWriter::Add(const char* str) {
	std::stringstream ss;
	ss << state_.prefix << " " << str;
	blueprint_.AddLine(ss.str());
}


} // namespace serial
