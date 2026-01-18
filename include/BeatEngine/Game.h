#pragma once

#include <memory>
#include <SFML/Window.hpp>

#include "BeatEngine/Managers.h"
#include "BeatEngine/Base/System.h"

#include "BeatEngine/View/ViewLayerStack.h"

#include "BeatEngine/Enum/AssetType.h"

/// <summary>
/// The base game class from the engine. This is a high level API and its the easist method to deploy a game using this class.
/// </summary>
class Game {
private:
	ViewManager* m_ViewMgr = nullptr;
	SystemManager* m_SystemMgr = nullptr;
	AssetManager* m_AssetMgr = nullptr;
	SettingsManager* m_SettingsMgr = nullptr;
	AudioManager* m_AudioMgr = nullptr;
	UIManager* m_UIMgr = nullptr;
private:
	sf::RenderWindow* m_Window = nullptr;
	sf::View m_View;
	sf::Clock m_Clock;

	ViewLayerStack m_GlobalLayers;

	bool m_UseImGui = false;
    bool m_Running = false;
	bool m_Preloaded = false;
    bool m_InFullscreen = false;
private:
	std::filesystem::path m_SettingsPath = "config.ini";
public:
	Game();
	~Game();
public:
	/// <summary>
	/// Runs and updates the game.
	/// </summary>
	void Run();
public:
	/// <summary>
	/// Registers a custom system onto the SystemManager.
	/// </summary>
	/// <typeparam name="TSystem">the custom system class</typeparam>
	template<typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void RegisterSystem() {
		m_SystemMgr->RegisterSystem<TSystem>();
	}
	/// <summary>
	/// Registers a view onto the ViewManger.
	/// </summary>
	/// <typeparam name="TView">the derivated view class</typeparam>
	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void RegisterView() {
		m_ViewMgr->RegisterView<TView>();
	}
	template<typename TSettings>
		requires(std::is_base_of_v<Base::Settings, TSettings>)
	void RegisterSettings() {
		m_SettingsMgr->RegisterSettingsData<TSettings>();
	}

	void UseImGui(bool show);
    void UseImGuiDocking(bool docking);
	sf::Window* GetWindow();
    SettingsManager* GetSettingsManager();


	template<typename TLayer>
		requires(std::is_base_of_v<ViewLayer, TLayer>)
	void AddGlobalLayer() {
		std::shared_ptr<TLayer> layer = std::make_shared<TLayer>(m_UIMgr, m_AssetMgr, m_SettingsMgr);

		m_GlobalLayers.AttachLayer(layer);
	}
    
    template<typename TLayer>
        requires(std::is_base_of_v<ViewLayer, TLayer>)
    std::shared_ptr<TLayer> GetGlobalLayer() {
        auto layer = m_GlobalLayers.GetLayer(typeid(TLayer));
        return std::static_pointer_cast<TLayer>(layer);
    }

	void SetWindowSize(sf::Vector2u size);
	void SetWindowTitle(std::string title);

	void PreloadSettings();

	void SaveSettings();

	template<typename TSettings>
		requires(std::is_base_of_v<Base::Settings, TSettings>)
	std::shared_ptr<TSettings> GetSettings() {
		auto base = m_SettingsMgr->GetSettings(typeid(TSettings));

		return std::static_pointer_cast<TSettings>(base);
	}

	void SetConfigPath(std::filesystem::path path);
public:
	/// <summary>
	/// Loads the global assets and saves it into the AssetManager.
	/// </summary>
	/// <param name="globalAssets">a map containing the asset type and the path of the asset</param>
	void LoadGlobalAssets(std::unordered_map<AssetType, std::vector<std::filesystem::path>> globalAssets);
public:
	/// <summary>
	/// Performs a display operation.
	/// </summary>
	void Display();
	/// <summary>
	/// Perfoms a draw operation.
	/// </summary>
	void Draw();
	/// <summary>
	/// Perfoms a update operation.
	/// </summary>
	void Update();
private:
	void ApplyBaseSettings();
private:
	void InitSettings();

	void InitUI();

	void InitAudio();
	/// <summary>
	/// Initializes the internal ViewManager.
	/// </summary>
	void InitViews();
	/// <summary>
	/// Initializes the internal SystemManager.
	/// </summary>
	void InitSystems();
	/// <summary>
	/// Initializes the internal AssetManager.
	/// </summary>
	void InitAssets();
	/// <summary>
	/// Initializes the SFML window with given settings parameters using the Settings class.
	/// </summary>
	void InitWindow();
	/// <summary>
	/// Initializes the internal KeybindManager.
	/// </summary>
	void InitKeybinds();
	/// <summary>
	/// Subscribes to the derivated game events.
	/// </summary>
	void SubscribeToGameEvent();
	/// <summary>
	/// Subscribes to the derivated game signals.
	/// </summary>
	void SubscribeToGameSignals();
};
