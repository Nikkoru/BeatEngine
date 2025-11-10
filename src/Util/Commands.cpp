#include "BeatEngine/Enum/Commands.h"

std::string CommandUtils::GetCommandString(const Command command) {
	if (_CommandString.contains(command))
		return _CommandString.at(command);
	else
		return "Unknown Command";
}

Command CommandUtils::GetCommandFromString(const std::string& str) {
	for (auto& pair : _CommandString) {
		if (str == pair.second)
			return pair.first;
	}
	return Command::Unknown; // Default fallback
}