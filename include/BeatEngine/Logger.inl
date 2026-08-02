#include "BeatEngine/Logger.h"
#include "BeatEngine/Enum/LogType.h"
#include "BeatEngine/System/Clock.h"
#include "BeatEngine/System/String.hpp"
#include "BeatEngine/System/Time.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iostream>
#include <format>
#include <print>
#include <string>

namespace {
template<typename T>
decltype(auto) prepareArg(T&& arg) {
    if constexpr (
		std::is_same_v<std::decay_t<T>, std::wstring> ||
		std::is_same_v<std::decay_t<T>, const wchar_t*> ||
		std::is_same_v<std::decay_t<T>, wchar_t*> ||
		std::is_same_v<std::decay_t<T>, std::string> ||
		std::is_same_v<std::decay_t<T>, const char*> ||
		std::is_same_v<std::decay_t<T>, char*>
    ) {
        return String(arg);
	}
    else {
        return std::forward<T>(arg);
    }
}

template<typename... Args>
std::wstring formatString(std::wstring_view fmt, Args&&... args) {
	auto preparedArgs = std::make_tuple(prepareArg(std::forward<Args>(args))...);

    return std::apply([&fmt](auto&... preparedArgs) {
        return std::vformat(fmt, std::make_wformat_args(preparedArgs...));
    }, preparedArgs);
}
}

template<typename... Args>
void Logger::AddLog(String logType, String caller, std::wstring_view fmt, Args&&... elms) {
    if (fmt == L"") return;   

    String preFormattedLog{};
    String formattedLog{};
    String savedLog{};

    String unescapedLogType = logType; 

    {
        bool colorEscape{ false };
        auto _ = std::remove_if(unescapedLogType.begin(), unescapedLogType.end(), [](unsigned char c) {
            return std::iscntrl(c);
        });
    }
        
    if constexpr (sizeof...(elms) > 0) {
        preFormattedLog = formatString(fmt, std::forward<Args>(elms)...);
    }
    else
        preFormattedLog = fmt;
    
    auto now = Clock::GetNow();
	auto nowT = now.AsTimeT(); 

#ifndef _WIN32
	std::tm* nowTM = std::localtime(&nowT);
	char nowStr[80];
	std::strftime(nowStr, sizeof(nowStr), "%T", nowTM);
#else
    std::tm nowTM;
	localtime_s(&nowTM, &nowT);
	char nowStr[80];
	std::strftime(nowStr, sizeof(nowStr), "%T", &nowTM);
#endif

    String nowStrW = nowStr;

	if (caller != "") {
		formattedLog = std::format(L"{} [{}] {} ({})", nowStrW, logType, preFormattedLog, caller);
        savedLog = std::format(L"{} [{}] {} ({})", nowStrW, unescapedLogType, preFormattedLog, caller);
	}
	else { 
		formattedLog = std::format(L"{} [{}] {}", nowStrW, logType, preFormattedLog);
        savedLog = std::format(L"{} [{}] {}", nowStrW, unescapedLogType, preFormattedLog);
    }

    std::println("{}", formattedLog);

	GetInstance()->m_Logs.push_back({ now.AsMicroseconds(), { LogTypeUtils::StringToType(logType), savedLog } });
}

template<typename... Args>
void Logger::AddLog(String logType, String caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(logType, caller, fmtW.ToWString(), elms...);
}

template<typename... Args>
void Logger::AddLog(LogType logType, String caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(std::format(L"{}{}\033[0m", GetColorViaLogType(logType), LogTypeUtils::TypeToString(logType)), caller, fmt, elms...);
}

template<typename... Args>
void Logger::AddLog(LogType logType, String caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(logType, caller, fmtW.ToWString(), elms...);
}


template<typename... Args>
void Logger::AddLog(String logType, std::type_index caller, std::wstring_view fmt, Args&&... elms) {
    String callerName = caller.name();
	if (caller == typeid(nullptr))
        callerName = "";
	else {
		if (callerName.Contains("class")) {
			callerName = callerName.SubString(callerName.find("class") + 6);
		}
		else if (callerName.Contains("struct"))
			callerName = callerName.SubString(callerName.find("struct") + 7);
		else
			callerName = caller.name();
	}
    AddLog(logType, callerName, fmt, elms...);
}

template<typename... Args>
void Logger::AddLog(String logType, std::type_index caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(logType, caller, fmtW.ToWString(), elms...);
}

template<typename... Args>
void Logger::AddLog(LogType logType, std::type_index caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(std::format(L"{}{}\033[0m", GetColorViaLogType(logType), LogTypeUtils::TypeToString(logType)), caller, fmt, elms...);
}

template<typename... Args>
void Logger::AddLog(LogType logType, std::type_index caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(logType, caller, fmtW.ToWString(), elms...);
}


template<typename... Args>
void Logger::AddDebug(String caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Debug, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddDebug(String caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Debug, caller, fmtW.ToWString(), elms...);
}
template<typename... Args>
void Logger::AddDebug(std::type_index caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Debug, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddDebug(std::type_index caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Debug, caller, fmtW.ToWString(), elms...);
}
template<typename... Args>
void Logger::AddInfo(String caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Info, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddInfo(String caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Info, caller, fmtW.ToWString(), elms...);
}
template<typename... Args>
void Logger::AddInfo(std::type_index caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Info, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddInfo(std::type_index caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Info, caller, fmtW.ToWString(), elms...);
}
template<typename... Args>
void Logger::AddWarning(String caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Warning, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddWarning(String caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Warning, caller, fmtW.ToWString(), elms...);
}
template<typename... Args>
void Logger::AddWarning(std::type_index caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Warning, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddWarning(std::type_index caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Warning, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddError(String caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Error, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddError(String caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Error, caller, fmtW.ToWString(), elms...);
}
template<typename... Args>
void Logger::AddError(std::type_index caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Error, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddError(std::type_index caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Error, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddCritical(String caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Critical, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddCritical(String caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Critical, caller, fmtW.ToWString(), elms...);
}
template<typename... Args>
void Logger::AddCritical(std::type_index caller, std::wstring_view fmt, Args&&... elms) {
    AddLog(LogType::Critical, caller, fmt, elms...);
}
template<typename... Args>
void Logger::AddCritical(std::type_index caller, std::string_view fmt, Args&&... elms) {
    String fmtW = fmt;
    AddLog(LogType::Critical, caller, fmt, elms...);
}
