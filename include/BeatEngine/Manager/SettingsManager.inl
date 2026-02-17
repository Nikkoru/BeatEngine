#include "BeatEngine/Manager/SettingsManager.h"

#include "BeatEngine/Logger.h"

template<typename TSettings>
    requires(std::is_base_of_v<Base::Settings, TSettings>)
void SettingsManager::RegisterSettingsData() {
    std::type_index ID = typeid(TSettings);

    if (m_Settings.contains(ID))
        Logger::AddWarning(typeid(SettingsManager), "{} is already registed", ID.name());
    else
        m_Settings.try_emplace(ID, std::make_shared<TSettings>());
}
