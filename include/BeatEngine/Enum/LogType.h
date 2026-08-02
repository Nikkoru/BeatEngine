#pragma once

#include "BeatEngine/System/String.hpp"
#include <cstdint>
#include <map>

enum class LogType : uint8_t {
	All = 0,
    Debug,
	Info,
	Warning,
	Error,
	Critical,
	DebugTarget,
	NoLogs = 99,
    Custom,
	None = 255
};

class LogTypeUtils {
public:
	static String TypeToString(const LogType type);
	static LogType StringToType(const String& str);
	static std::map<LogType, String> GetTypeStringMap() { return _TypeString; }
private:
	static inline const std::map<LogType, String> _TypeString = {
        { LogType::All, "All" },
        { LogType::Debug, "Debug" },
        { LogType::Info, "Info" },
        { LogType::Warning, "Warning" },
        { LogType::Error, "Error" },
        { LogType::Critical, "Critical" },
        { LogType::DebugTarget, "Debug Target" },
        { LogType::NoLogs, "No Logs" },
        { LogType::None, "None"}
	};
};
