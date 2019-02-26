#pragma once

namespace serial {

class BasicRef {
public:
	BasicRef() = default;
	BasicRef(nullptr_t) {}

	BasicRef(ReferableBase* ref) {
		ref_ = ref;
	}

	BasicRef& operator=(nullptr_t) {
		ref_ = nullptr;
		return *this;
	}

	BasicRef& operator=(ReferableBase* u) {
		ref_ = u;
		return *this;
	}

	ReferableBase* Get() {
		return ref_;
	}

	const ReferableBase* Get() const {
		return ref_;
	}

	template<typename T>
	bool Is() const {
		if (!ref_) {
			return false;
		}
		return ref_->HasType<T>();
	}

	bool operator==(const BasicRef& other) const {
		return ref_ == other.ref_;
	}

	bool operator!=(const BasicRef& other) const {
		return ref_ != other.ref_;
	}

	explicit operator bool() const {
		return ref_ != nullptr;
	}

private:
	ReferableBase* ref_ = nullptr;
};

} // namespace serial
