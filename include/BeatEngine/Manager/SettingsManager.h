#pragma once

#include "BeatEngine/Base/Settings.h"
#include "BeatEngine/Logger.h"

#include <string>
#include <filesystem>
#include <map>
#include <typeindex>

namespace fs = std::filesystem;

class Game;
class SettingsManager {
private:
	std::map<std::type_index, std::shared_ptr<Base::Settings>> m_Settings;
public:
	SettingsManager() = default;
	~SettingsManager() = default;
private:
	friend class Game;
	void ReadConfig(fs::path path);
	void WriteConfig(fs::path path);
public:
	template<typename TSettings>
		requires(std::is_base_of_v<Base::Settings, TSettings>)
	void RegisterSettingsData() {
		std::type_index ID = typeid(TSettings);

		if (m_Settings.contains(ID))
			Logger::GetInstance()->AddWarning(std::format("{} is already registed", ID.name()), typeid(SettingsManager));
		else
			m_Settings.try_emplace(ID, std::make_shared<TSettings>());
	}

	std::shared_ptr<Base::Settings> GetSettings(std::string tag);
	std::shared_ptr<Base::Settings> GetSettings(std::type_index id);

	void SetSettings(std::type_index settingsID, std::shared_ptr<Base::Settings> settings);
	void SetSettings(std::string tag, std::shared_ptr<Base::Settings> settings);
private:
	char* GetTextData(fs::path path);
};