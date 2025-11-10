#include "BeatEngine/Base/View.h"

bool Base::View::operator==(const View& other) const {
	return this->ID == other.ID;
}

void Base::View::OnSuspend() {
	m_Suspended = true;
}

void Base::View::OnResume() {
	m_Suspended = false;
}

bool Base::View::IsSuspended() const {
	return m_Suspended;
}