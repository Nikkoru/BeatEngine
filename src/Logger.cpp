#include "BeatEngine/Logger.h"
#include "BeatEngine/Enum/LogType.h"

#include <utility>
#include <ctime>

std::shared_ptr<Logger> Logger::m_Instance = nullptr;

std::string Logger::GetColorViaLogType(LogType logType) {
    const std::string red = "\e[0;31m";
    const std::string yellow = "\e[0;33m";
    const std::string purple = "\e[0;35m";
    const std::string cyan = "\e[0;36m";
    const std::string hiRed = "\e[0;91m";

    std::string color;

    switch (logType) {
        case LogType::Custom:
        case LogType::None:
        case LogType::All:
        case LogType::NoLogs:
        case LogType::Info:
            color = cyan;
            break;
        case LogType::Warning:
            color = yellow;
            break;
        case LogType::Error:
            color = red;
            break;
        case LogType::Critical:
            color = hiRed;
            break;
        case LogType::DebugTarget:
            color = purple;
            break;
    }

    return color;
}

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
