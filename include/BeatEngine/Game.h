#pragma once

#include <memory>

#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Managers.h"
#include "BeatEngine/Base/System.h"
#include "BeatEngine/Enum/GameFlags.h"

#include "BeatEngine/System/Clock.h"
#include "BeatEngine/View/ViewLayerStack.h"

#include "BeatEngine/Enum/AssetType.h"

/// <summary>
/// The base game class from the engine. This is a high level API and its the easist method to deploy a game using this class.
/// </summary>
class Game {
private:
	Clock m_Clock{};
    // sf::Cursor m_Cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();

	ViewLayerStack m_GlobalLayers{};
    
    std::shared_ptr<GameContext> m_Context{ nullptr };
    std::shared_ptr<GameState> m_State{ nullptr };
    bool m_Running = false;
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
    void Initialize();
    void Uninitialize();
public:
    /// <summary>
	/// Registers a custom system onto the SystemManager.
	/// </summary>
	/// <typeparam name="TSystem">the custom system class</typeparam>
	template<typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void RegisterSystem();
	/// <summary>
	/// Registers a view onto the ViewManger.
	/// </summary>
	/// <typeparam name="TView">the derivated view class</typeparam>
	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void RegisterView();

	template<typename TSettings>
		requires(std::is_base_of_v<Base::Settings, TSettings>)
	void RegisterSettings();

    template<typename TRenderer>
        requires(std::is_base_of_v<Renderer, TRenderer>)
    void SetRenderer();

    void SetRenderer(std::shared_ptr<Renderer> renderer);

	void UseImGui(bool show);
    void UseImGuiDocking(bool docking);

	template<typename TLayer>
		requires(std::is_base_of_v<ViewLayer, TLayer>)
	void AddGlobalLayer();
    
    template<typename TLayer>
        requires(std::is_base_of_v<ViewLayer, TLayer>)
    std::shared_ptr<TLayer> GetGlobalLayer();

	void SetWindowSize(Vector2u size);
	void SetWindowTitle(std::string title);

	void PreloadSettings();

	void SaveSettings();

	template<typename TSettings>
		requires(std::is_base_of_v<Base::Settings, TSettings>)
	std::shared_ptr<TSettings> GetSettings();

	void SetConfigPath(std::filesystem::path path);
    void SetFlags(GameFlags flags);
    void RemoveFlags(GameFlags flags);

    void DrawImGuiDebug();
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

#include "BeatEngine/Game.inl"
