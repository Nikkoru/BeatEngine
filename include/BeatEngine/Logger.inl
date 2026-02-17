#include "BeatEngine/Enum/LogType.h"
#include "BeatEngine/Logger.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <ctime>
#include <chrono>
#include <iostream>
#include <format>
#include <string>

template<typename... Args>
void Logger::AddLog(std::string logType, std::string caller, std::string_view fmt, Args&&... elms) {
    if (fmt == "") return;   

    std::string preFormattedLog{};
    std::string formattedLog{};
    std::string savedLog{};

    auto unescapedLogType = logType; 
    auto unescapedLogTypeIt = std::remove_if(unescapedLogType.begin(), unescapedLogType.end(), [](unsigned char c) {
        return std::iscntrl(c);
    });
    unescapedLogType.erase(unescapedLogTypeIt, unescapedLogType.end());
    
    if constexpr (sizeof...(elms) > 0) {
        std::format_args fmt_elms = std::make_format_args(elms...);
        preFormattedLog = std::vformat(fmt, fmt_elms);
    }
    else
        preFormattedLog = fmt;

	auto nowTp = std::chrono::system_clock::now();
	std::time_t nowT = std::chrono::system_clock::to_time_t(nowTp);

	std::tm* now = std::localtime(&nowT);
	char nowStr[80];
	std::strftime(nowStr, sizeof(nowStr), "%T", now);

	if (caller != "") {
		formattedLog = std::format("{} [{}] {} ({})", nowStr, logType, preFormattedLog, caller);
        savedLog = std::format("{} [{}] {} ({})", nowStr, unescapedLogType, preFormattedLog, caller);
	}
	else { 
		formattedLog = std::format("{} [{}] {}", nowStr, logType, preFormattedLog);
        savedLog = std::format("{} [{}] {}", nowStr, unescapedLogType, preFormattedLog);
    }

    std::cout << formattedLog << std::endl;

	GetInstance()->m_Logs.push_back({ nowT, { LogTypeUtils::StringToType(logType), savedLog } });
}
template<typename... Args>
void Logger::AddLog(LogType logType, std::string caller, std::string_view fmt, Args&&... elms) {
    AddLog(std::format("{}{}\033[0m", GetColorViaLogType(logType), LogTypeUtils::TypeToString(logType)), caller, fmt, elms...);
}

template<typename... Args>
void Logger::AddLog(std::string logType, std::type_index caller, std::string_view fmt, Args&&... elms) {
	if (caller == typeid(nullptr))
		AddLog(logType, "", fmt, elms...);
	else {
		std::string callerName = caller.name();
		if (callerName.contains("class")) {
			callerName = callerName.substr(callerName.find("class") + strlen("class") + 1);
		}
		else if (callerName.contains("struct"))
			callerName = callerName.substr(callerName.find("struct") + strlen("struct") + 1);
		else
			callerName = caller.name();
		AddLog(logType, callerName, fmt, elms...);
	}
}

template<typename... Args>
void Logger::AddLog(LogType logType, std::type_index caller, std::string_view fmt, Args&&... elms) {
    AddLog(std::format("{}{}\033[0m", GetColorViaLogType(logType), LogTypeUtils::TypeToString(logType)), caller, fmt, elms...);
}

template<typename... Args>
void Logger::AddInfo(std::string caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Info, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddInfo(std::type_index caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Info, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddWarning(std::string caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Warning, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddWarning(std::type_index caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Warning, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddError(std::string caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Error, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddError(std::type_index caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Error, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddCritical(std::string caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Critical, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddCritical(std::type_index caller, std::string_view fmt, Args&&... elms) {
    AddLog(LogType::Critical, caller, fmt, elms...);
}
