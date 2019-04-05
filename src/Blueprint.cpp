#include <sstream>
#include <algorithm>
#include <iterator>
#include "serial/Blueprint.h"


namespace serial {

// Blueprint

Blueprint Blueprint::FromString(const std::string& str) {
	Blueprint bp;
	std::stringstream ss(str);
	std::string line;
	while (std::getline(ss, line, '\n')) {
		if (line.size() > 0) {
			bp.lines_.insert(line);
		}
	}
	return bp;
}

bool Blueprint::AddLine(const std::string& line) {
	if (line.find('\n') != std::string::npos) {
		return false;
	}

	lines_.insert(line);
	return true;
}

std::string Diff(const Blueprint& lhs, const Blueprint& rhs) {
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
		ss << "\n";
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

	return ss.str();
}

std::ostream& operator<<(std::ostream& stream, const Blueprint& bp) {
	for (auto& l : bp.lines_) {
		stream << l << std::endl;
	}
	return stream;
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

} // namespace serial
