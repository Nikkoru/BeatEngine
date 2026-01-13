#include "BeatEngine/Logger.h"
#include "BeatEngine/Enum/LogType.h"
#include "BeatEngine/Util/ChronoHelper.h"

#include <iostream>
#include <cstdarg>
#include <format>
#include <utility>
#include <chrono>
#include <ctime>

std::shared_ptr<Logger> Logger::m_Instance = nullptr;

std::shared_ptr<Logger> Logger::GetInstance() {
	 if (m_Instance == nullptr) 
		 m_Instance = std::make_shared<Logger>(); 
	 return m_Instance;
}

std::vector<std::pair<std::time_t, std::pair<LogType, std::string>>> Logger::GetLogs() const {
	return m_Logs;
}

void Logger::ClearLogs() {
	m_Logs.clear();
}
