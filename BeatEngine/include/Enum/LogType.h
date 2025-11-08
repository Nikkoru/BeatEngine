#pragma once

#include <cstdint>
#include <string>
#include <map>

enum class LogType : uint8_t {
	All = 0,
	Info,
	Warning,
	Error,
	Critical,
	DebugTarget,

	NoLogs = 99,
	Command,
	CommandInfo,
	CommandWarning,
	CommandError,
	None = 255
};

class LogTypeUtils {
public:
	static std::string TypeToString(const LogType type);
	static LogType StringToType(const std::string& str);
	static std::map<LogType, std::string> GetTypeStringMap() { return _TypeString; }
private:
	static inline const std::map<LogType, std::string> _TypeString = {
	{ LogType::All, "All" },
	{ LogType::Info, "Info" },
	{ LogType::Warning, "Warning" },
	{ LogType::Error, "Error" },
	{ LogType::NoLogs, "No Logs" },
	{ LogType::None, "None"},
	{ LogType::DebugTarget, "Debug Target" }
	};
};