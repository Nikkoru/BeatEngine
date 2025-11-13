#include "BeatEngine/Logger.h"
#include "BeatEngine/Enum/LogType.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <cstdarg>
#include <format>
#include <utility>

std::shared_ptr<Logger> Logger::m_Instance = nullptr;

std::shared_ptr<Logger> Logger::GetInstance() {
	 if (m_Instance == nullptr) 
		 m_Instance = std::make_shared<Logger>(); 
	 return m_Instance;
}

void Logger::AddLog(std::string log, LogType logType, std::string caller) {
	std::string formattedLog;

	auto nowTp = std::chrono::system_clock::now();
	std::time_t nowTime_t = std::chrono::system_clock::to_time_t(nowTp);
	std::tm* now = std::localtime(&nowTime_t);
	char nowStr[80];
	std::strftime(nowStr, sizeof(nowStr), "%T", now);

	if (caller != "") {
		formattedLog = std::format("{} [{}] {} ({})", nowStr, LogTypeUtils::TypeToString(logType), log, caller);
	}
	else
		formattedLog = std::format("{} [{}] {}", nowStr, LogTypeUtils::TypeToString(logType), log);
	if (logType == LogType::Error)
		std::cerr << formattedLog << std::endl;
	else
		std::cout << formattedLog << std::endl;

	m_Logs.push_back({ nowTime_t, { logType, formattedLog } });
}

void Logger::AddLog(std::string log, LogType logType, std::type_index caller) {
	if (caller == typeid(nullptr))
		AddLog(log, logType);
	else {
		std::string callerName = caller.name();
		if (callerName.contains("class")) {
			callerName = callerName.substr(callerName.find("class") + strlen("class") + 1);
		}
		else if (callerName.contains("struct"))
			callerName = callerName.substr(callerName.find("struct") + strlen("struct") + 1);
		else
			callerName = caller.name();
		AddLog(log, logType, callerName);
	}
}

void Logger::AddInfo(std::string log, std::string caller) {
	AddLog(log, LogType::Info, caller);
}

void Logger::AddInfo(std::string log, std::type_index caller) {
	AddLog(log, LogType::Info, caller);
}

void Logger::AddWarning(std::string log, std::string caller) {
	AddLog(log, LogType::Warning, caller);
}

void Logger::AddWarning(std::string log, std::type_index caller) {
	AddLog(log, LogType::Warning, caller);
}

void Logger::AddError(std::string log, std::string caller) {
	AddLog(log, LogType::Error, caller);
}

void Logger::AddError(std::string log, std::type_index caller) {
	AddLog(log, LogType::Error, caller);
}

void Logger::AddCritical(std::string log, std::string caller) {
	AddLog(log, LogType::Critical, caller);
}

void Logger::AddCritical(std::string log, std::type_index caller) {
	AddLog(log, LogType::Critical, caller);
}

std::vector<std::pair<std::time_t, std::pair<LogType, std::string>>> Logger::GetLogs() const {
	return m_Logs;
}

void Logger::ClearLogs() {
	m_Logs.clear();
}