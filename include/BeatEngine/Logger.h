#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <typeindex>
#include <imgui.h>
#include <memory>

#include "BeatEngine/Enum/Commands.h"
#include "BeatEngine/Enum/LogType.h"

class Logger {
public:
	Logger() = default;

	static std::shared_ptr<Logger> GetInstance();

	void AddLog(std::string log, LogType logType, std::string caller = "");
	void AddLog(std::string log, LogType logType, std::type_index caller);

	void AddInfo(std::string log, std::string caller = "");
	void AddInfo(std::string log, std::type_index caller);
	void AddWarning(std::string log, std::string caller = "");
	void AddWarning(std::string log, std::type_index caller); 
	void AddError(std::string log, std::string caller = "");
	void AddError(std::string log, std::type_index caller); 
	void AddCritical(std::string log, std::string caller = "");
	void AddCritical(std::string log, std::type_index caller);

	std::vector<std::pair<std::tm*, std::pair<LogType, std::string>>> GetLogs() const;

	void ClearLogs();
public:
	std::vector<std::pair<std::tm*, std::pair<LogType, std::string>>> m_Logs;
private:
	static std::shared_ptr<Logger> m_Instance;
};

