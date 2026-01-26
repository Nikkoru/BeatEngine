#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Manager/AudioManager.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Events/ViewEvent.h"
#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/UIManager.h"

#include <functional>
#include <memory>
#include <typeindex>
#include <stack>
#include <unordered_map>

class ViewManager {
public:
	using FabricCallback = std::function<std::unique_ptr<Base::View>(AssetManager*, SettingsManager*, AudioManager*, UIManager*)>;
public:
	std::unordered_map<std::type_index, FabricCallback> ViewFabrics;
	std::stack<std::unique_ptr<Base::View>> ViewStack;
	std::type_index MainView;
private:
	AssetManager* m_GlobalViewAssetMgr = nullptr;
	AudioManager* m_GlobalViewAudioMgr = nullptr;
	SettingsManager* m_GlobalViewSettingsMgr = nullptr;
	UIManager* m_GlobalViewUIMgr = nullptr;
private:
    GameContext* m_Context = nullptr;
public:
	ViewManager(GameContext* context);
	~ViewManager() = default;
public:
	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void Push() {
		Push(std::type_index(typeid(TView)));
	}
	void Push(std::type_index viewID);

	void Pop();

	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void RegisterView() {
		auto ID = std::type_index(typeid(TView));
		FabricCallback fabric = ([](AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr)
			-> std::unique_ptr<Base::View> { return std::make_unique<TView>(assetMgr, settingsMgr, audioMgr, uiMgr); });

		Logger::AddInfo(typeid(ViewManager), "Registing {}", typeid(TView).name());

		bool firstView = ViewFabrics.empty();

		if (ViewFabrics.find(ID) == ViewFabrics.end()) {
			ViewFabrics.try_emplace(ID, fabric);
			if (firstView)
				MainView = ID;
		}
	}

	bool OnSFMLEvent(std::optional<sf::Event> event);
	bool OnDraw(sf::RenderWindow* window);
	bool OnUpdate(float dt);
	bool OnExit();

	bool HasActiveViews();

	void GetViewKeybinds();

	void SetGlobalAssetManager(AssetManager* assetMgr);
    void SetGlobalAudioManager(AudioManager* audioMgr);
    void SetGlobalSettingsManager(SettingsManager* settingsMgr);
    void SetGlobalUIManager(UIManager* uiMgr);
public:
    void ShowImGuiDebugData();
};
