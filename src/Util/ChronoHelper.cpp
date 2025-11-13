#include "BeatEngine/Util/ChronoHelper.h"

unsigned int ChronoHelper::GetCurrentTimeMiliseconds() {
	auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

unsigned int ChronoHelper::TimePointToMiliseconds(const std::chrono::system_clock::time_point& timePoint) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count();
}