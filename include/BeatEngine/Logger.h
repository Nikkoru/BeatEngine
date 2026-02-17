#pragma once

#include <string>
#include <vector>
#include <typeindex>
#include <memory>
#include <ctime>

#include "BeatEngine/Enum/LogType.h"

class Logger {
private:
    static std::string GetColorViaLogType(LogType logType);
public:
	Logger() = default;

	static std::shared_ptr<Logger> GetInstance();
   
    template<typename... Args>
	static void AddLog(std::string logType, std::string caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddLog(LogType logType, std::string caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddLog(std::string logType, std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddLog(LogType logType, std::type_index caller, std::string_view fmt = "", Args&&... elms);

    template<typename... Args>
	static void AddInfo(std::string caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddInfo(std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddWarning(std::string caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddWarning(std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddError(std::string caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddError(std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddCritical(std::string caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddCritical(std::type_index caller, std::string_view fmt = "", Args&&... elms);

	std::vector<std::pair<std::time_t, std::pair<LogType, std::string>>> GetLogs() const;

	void ClearLogs();
public:
	std::vector<std::pair<std::time_t, std::pair<LogType, std::string>>> m_Logs;
private:
	static std::shared_ptr<Logger> m_Instance;
};

#include "BeatEngine/Logger.inl"
