#include "BeatEngine/Logger.h"
#include "BeatEngine/Enum/LogType.h"

#include <utility>
#include <ctime>

std::shared_ptr<Logger> Logger::m_Instance = nullptr;

std::wstring Logger::GetColorViaLogType(LogType logType) {
    const std::wstring red = L"\033[0;31m";
    const std::wstring yellow = L"\033[0;33m";
    const std::wstring purple = L"\033[0;35m";
    const std::wstring cyan = L"\033[0;36m";
    const std::wstring blue = L"\033[0;34m";
    const std::wstring hiRed = L"\033[0;91m";

    std::wstring color{};

    switch (logType) {
        default:
        case LogType::Custom:
        case LogType::None:
        case LogType::All:
        case LogType::NoLogs:
        case LogType::Debug:
            color = blue;
            break;
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

std::vector<std::pair<std::time_t, std::pair<LogType, String>>> Logger::GetLogs() const {
	return m_Logs;
}

void Logger::ClearLogs() {
	m_Logs.clear();
}
