#pragma once

#include <cstdint>
#include <map>
#include <string>

enum class Commands : uint8_t {
	SetFpsLimit,
	Unknown = 255
};

class CommandUtils {
public:
	static std::string GetCommandString(const Commands command);
	static Commands GetCommandFromString(const std::string& str);
	static std::map<Commands, std::string> GetStringMap() { return _CommandString; }
private:
	static inline const std::map<Commands, std::string> _CommandString = {
		{ Commands::SetFpsLimit, "setFpsLimit" }
	};
};