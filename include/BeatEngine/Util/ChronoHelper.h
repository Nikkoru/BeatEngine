#pragma once

#include <chrono>

namespace ChronoHelper {
	unsigned int GetCurrentTimeMiliseconds();
	unsigned int TimePointToMiliseconds(const std::chrono::system_clock::time_point& timePoint);
}