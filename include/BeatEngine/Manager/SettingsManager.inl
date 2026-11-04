#include "BeatEngine/Manager/SettingsManager.h"

#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Exception.h"
#include <memory>
#include <type_traits>
#include <typeindex>

template<typename TSettings>
    requires(std::is_base_of_v<Base::Settings, TSettings>)
void SettingsManager::RegisterSettingsData() {
    std::type_index ID = typeid(TSettings);

    if (m_Settings.contains(ID))
        Logger::AddWarning(typeid(SettingsManager), "{} is already registed", ID.name());
    else
        m_Settings.try_emplace(ID, std::make_shared<TSettings>());
}

template<typename TSettings>
    requires(std::is_base_of_v<Base::Settings, TSettings>)
bool SettingsManager::HasSettings() {
    return m_Settings.contains(typeid(TSettings));
}

template<typename TSettings>
    requires(std::is_base_of_v<Base::Settings, TSettings>)
void SettingsManager::SetSettings(std::type_index settingsID, const TSettings& settings) {
	for (const auto& [id, _] : m_Settings) {
		if (id == settingsID) {
			m_Settings.try_emplace(id, std::make_shared<TSettings>(settings));
			return;
		}
	}

	std::string msg = std::format("Unable to get: \"{}\"", settingsID.name());
	Logger::AddCritical(typeid(SettingsManager), msg);
	THROW_RUNTIME_ERROR(msg);
}

template<typename TSettings>
    requires(std::is_base_of_v<Base::Settings, TSettings>)
void SettingsManager::SetSettings(std::string tag, const TSettings& settings) {
	for (auto& [id, _] : m_Settings) {
		if (tag == settings.GetTag()) {
			m_Settings.try_emplace(id, std::make_shared<TSettings>(settings));
			return;
		}
	}

	std::string msg = "Unable to get settings with tag: \"" + tag + "\"";
	Logger::AddCritical(typeid(SettingsManager), msg);
	THROW_RUNTIME_ERROR(msg);
}
