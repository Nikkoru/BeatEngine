#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/SettingsSignals.h"

#include <iostream>
#include <fstream>
#include <memory>

SettingsManager::SettingsManager() {
    SignalManager::GetInstance()->RegisterCallback<SetSettingsSignal>(typeid(SettingsManager), [this](const std::shared_ptr<Base::Signal> signal) {
        auto setSignal = std::static_pointer_cast<SetSettingsSignal>(signal);

        SetSettings(setSignal->SettingsID, setSignal->Settings);

        EventManager::GetInstance()->Send(std::make_shared<GameSettingsChanged>());
    });
}

void SettingsManager::ReadConfig(fs::path path) {
	Logger::AddInfo(typeid(SettingsManager), "Reading config file \"{}\"", path.stem().string());
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
		Logger::AddWarning(typeid(SettingsManager), "File dosen't exists, creating");
		std::ofstream f(path);
		f.close();
	}
	delete[] iniData;
}

void SettingsManager::WriteConfig(fs::path path) {
	Logger::AddInfo(typeid(SettingsManager), "Writing config");

	std::ofstream file(path);

	if (file.is_open()) {
		for (auto& [id, settings] : m_Settings)
			file << settings->Write() << std::endl;
	}
	file.close();
}

std::shared_ptr<Base::Settings> SettingsManager::GetSettings(std::string tag) {
	for (auto& [id, settings] : m_Settings) {
		if (tag == settings->GetTag())
			return settings;
	}

	std::string msg = "Unable to get settings with tag: \"" + tag + "\"";
	Logger::AddCritical(typeid(SettingsManager), msg);
	THROW_RUNTIME_ERROR(msg);
}

std::shared_ptr<Base::Settings> SettingsManager::GetSettings(std::type_index settingsID) {
	for (auto& [id, settings] : m_Settings) {
		if (id == settingsID)
			return settings;
	}

	std::string msg = std::format("Unable to get: \"{}\"", settingsID.name());
	Logger::AddCritical(typeid(SettingsManager), msg);
	THROW_RUNTIME_ERROR(msg);
}

void SettingsManager::SetSettings(std::type_index settingsID, std::shared_ptr<Base::Settings> settings) {
	for (auto& [id, _] : m_Settings) {
		if (id == settingsID) {
			m_Settings.try_emplace(id, settings);
			return;
		}
	}

	std::string msg = std::format("Unable to get: \"{}\"", settingsID.name());
	Logger::AddCritical(typeid(SettingsManager), msg);
	THROW_RUNTIME_ERROR(msg);
}

void SettingsManager::SetSettings(std::string tag, std::shared_ptr<Base::Settings> settings) {
	for (auto& [id, _] : m_Settings) {
		if (tag == settings->GetTag()) {
			m_Settings.try_emplace(id, settings);
			return;
		}
	}

	std::string msg = "Unable to get settings with tag: \"" + tag + "\"";
	Logger::AddCritical(typeid(SettingsManager), msg);
	THROW_RUNTIME_ERROR(msg);
}

void SettingsManager::SetDefaults() {
	Logger::AddInfo(typeid(SettingsManager), "Setting defaults...");

	for (auto& [index, settings] : m_Settings)
		settings->SetDefaults();
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
