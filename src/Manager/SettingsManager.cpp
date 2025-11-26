#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Util/Exception.h"

#include <iostream>
#include <fstream>

void SettingsManager::ReadConfig(fs::path path) {
	Logger::GetInstance()->AddInfo("Reading config file \"" + path.stem().string() + "\"", typeid(SettingsManager));
	const char* iniData = GetTextData(path);
	if (iniData != NULL) {
		size_t iniSize = strlen(iniData);

		char* const buf = (char*)malloc(iniSize);
		char* const bufEnd = buf + iniSize;

		std::shared_ptr<Base::Settings> settings = nullptr;

		memcpy(buf, iniData, iniSize);

		char* endLine = NULL;

		for (char* line = buf; line < bufEnd; line = endLine + 1) {
			while (*line == '\n' || *line == '\r')
				line++;
			endLine = line;
			while (endLine < bufEnd && *endLine != '\n' && *endLine != '\r')
				endLine++;
			endLine[0] = 0;
			if (line[0] == '[' && endLine > line && endLine[-1] == ']') {
				settings = GetSettings(line);
			}
			else if (settings != nullptr) {
				settings->Read(line);
			}
		}

		free(buf);
	}
	else {
		Logger::GetInstance()->AddWarning("File dosen't exists, creating", typeid(SettingsManager));
		std::ofstream f(path);
		f.close();
	}
	delete[] iniData;
}

void SettingsManager::WriteConfig(fs::path path) {
	std::ofstream file(path);

	if (file.is_open()) {
		for (auto& [id, settings] : m_Settings)
			file << settings->Write() << std::endl;
		file << "a";
	}
	file.close();
}

std::shared_ptr<Base::Settings> SettingsManager::GetSettings(std::string tag) {
	for (auto& [id, settings] : m_Settings) {
		if (tag == settings->GetTag())
			return settings;
	}

	std::string msg = "Unable to get settings with tag: \"" + tag + "\"";
	Logger::GetInstance()->AddCritical(msg, typeid(SettingsManager));
	THROW_RUNTIME_ERROR(msg);
}

std::shared_ptr<Base::Settings> SettingsManager::GetSettings(std::type_index settingsID) {
	for (auto& [id, settings] : m_Settings) {
		if (id == settingsID)
			return settings;
	}

	std::string msg = std::format("Unable to get: \"{}\"", settingsID.name());
	Logger::GetInstance()->AddCritical(msg, typeid(SettingsManager));
	THROW_RUNTIME_ERROR(msg);
}

char* SettingsManager::GetTextData(fs::path path) {
	std::ifstream file(path);

	file.ignore(std::numeric_limits<std::streamsize>::max());
	const std::streamsize fileSize = file.gcount();
	file.clear();
	file.seekg(0, std::ios::beg);

	std::vector<char> buf(fileSize);

	char* bufChar = new char[fileSize];

	if (file.read(buf.data(), fileSize)) {
		memcpy(bufChar, buf.data(), fileSize);
	}
	else {
		delete[] bufChar;
		return nullptr;
	}

	return bufChar;
}
