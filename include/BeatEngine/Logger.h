#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <typeindex>
#include <memory>
#include <ctime>

#include "BeatEngine/Enum/LogType.h"
#include "BeatEngine/System/String.hpp"

class Logger {
private:
    static std::wstring GetColorViaLogType(LogType logType);
public:
	Logger() = default;

	static std::shared_ptr<Logger> GetInstance();
   
    template<typename... Args>
	static void AddLog(String logType, String caller = "", std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddLog(String logType, String caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddLog(LogType logType, String caller = "", std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddLog(LogType logType, String caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddLog(String logType, std::type_index caller, std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddLog(String logType, std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddLog(LogType logType, std::type_index caller, std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddLog(LogType logType, std::type_index caller, std::string_view fmt = "", Args&&... elms);

    template<typename... Args>
    static void AddDebug(String caller = "", std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
    static void AddDebug(String caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
    static void AddDebug(std::type_index caller, std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
    static void AddDebug(std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddInfo(String caller = "", std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddInfo(String caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddInfo(std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddInfo(std::type_index caller, std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddWarning(String caller = "", std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddWarning(std::type_index caller, std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddWarning(String caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddWarning(std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddError(String caller = "", std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddError(std::type_index caller, std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddError(String caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddError(std::type_index caller, std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddCritical(String caller = "", std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddCritical(std::type_index caller, std::wstring_view fmt = L"", Args&&... elms);
    template<typename... Args>
	static void AddCritical(String caller = "", std::string_view fmt = "", Args&&... elms);
    template<typename... Args>
	static void AddCritical(std::type_index caller, std::string_view fmt = "", Args&&... elms);

	std::vector<std::pair<std::time_t, std::pair<LogType, String>>> GetLogs() const;

	void ClearLogs();
public:
	std::vector<std::pair<std::time_t, std::pair<LogType, String>>> m_Logs;
private:
	static std::shared_ptr<Logger> m_Instance;
};

#include "BeatEngine/Logger.inl"
