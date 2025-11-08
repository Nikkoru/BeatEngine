#include "Enum/Commands.h"

std::string CommandUtils::GetCommandString(const Commands command) {
	if (_CommandString.contains(command))
		return _CommandString.at(command);
	else
		return "Unknown Command";
}

Commands CommandUtils::GetCommandFromString(const std::string& str) {
	for (auto& pair : _CommandString) {
		if (str == pair.second)
			return pair.first;
	}
	return Commands::Unknown; // Default fallback
}