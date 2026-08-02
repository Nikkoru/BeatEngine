#include "BeatEngine/Enum/LogType.h"

String LogTypeUtils::TypeToString(const LogType type) {
	if (_TypeString.contains(type))
		return _TypeString.at(type);
	else
		return _TypeString.at(LogType::None);
}

LogType LogTypeUtils::StringToType(const String& str) {
	for (auto& pair : _TypeString) {
		if (str == pair.second)
			return pair.first;
	}
	return LogType::Custom;
}
