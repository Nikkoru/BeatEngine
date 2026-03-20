#include "BeatEngine/Base/View.h"

bool Base::View::operator==(const View& other) const {
	return this->b_ID == other.b_ID;
}

void Base::View::OnSuspend() {
	b_mSuspended = true;
}

void Base::View::OnResume() {
	b_mSuspended = false;
}

bool Base::View::IsSuspended() const {
	return b_mSuspended;
}
