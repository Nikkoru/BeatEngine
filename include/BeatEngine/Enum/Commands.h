#pragma once

#include <cstdint>
#include <map>
#include <string>

enum class Command : uint8_t {
	SetFpsLimit,
	Unknown = 255
};

class CommandUtils {
public:
	static std::string GetCommandString(const Command command);
	static Command GetCommandFromString(const std::string& str);
	static std::map<Command, std::string> GetStringMap() { return _CommandString; }
private:
	static inline const std::map<Command, std::string> _CommandString = {
		{ Command::SetFpsLimit, "setFpsLimit" }
	};
};