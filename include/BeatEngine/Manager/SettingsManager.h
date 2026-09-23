#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <map>
#include <type_traits>
#include <typeindex>

#include "BeatEngine/Manager/SignalManager.h"

namespace fs = std::filesystem;

namespace Base {
    class Settings;
};
class Application;
class AppContext;
class AppState;
class SettingsManager {
private:
	std::map<std::type_index, std::shared_ptr<Base::Settings>> m_Settings;
private:
    AppContext* m_Context{ nullptr };
    AppState* m_State{ nullptr };
public:
    SettingsManager() : SettingsManager(nullptr, nullptr) {}
	SettingsManager(AppContext* context, AppState* state); 
	~SettingsManager() { SignalManager::GetInstance()->RemoveCallbacks(typeid(SettingsManager)); };
private:
	friend class Application;
	void ReadConfig(fs::path path);
	void WriteConfig(fs::path path);
public:
    void SetContext(AppContext* context) { m_Context = context; }
    void SetState(AppState* state) { m_State = state; }

	template<typename TSettings>
		requires(std::is_base_of_v<Base::Settings, TSettings>)
	void RegisterSettingsData(); 
    std::shared_ptr<Base::Settings> GetSettings(std::string tag);
    std::shared_ptr<Base::Settings> GetSettings(std::type_index id);

    template<typename TSettings>
        requires(std::is_base_of_v<Base::Settings, TSettings>)
    bool HasSettings();

	void SetSettings(std::type_index settingsID, std::shared_ptr<Base::Settings> settings);
	void SetSettings(std::string tag, std::shared_ptr<Base::Settings> settings);

	void SetDefaults();
public:
    void ShowImGuiDebugWindow();
private:
	char* GetTextData(fs::path path);
};

#include "BeatEngine/Manager/SettingsManager.inl"
