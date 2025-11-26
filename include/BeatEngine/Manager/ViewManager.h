#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Events/ViewEvent.h"
#include "BeatEngine/Manager/AssetManager.h"

#include <functional>
#include <memory>
#include <typeindex>
#include <stack>
#include <unordered_map>

class ViewManager {
public:
	using FabricCallback = std::function<std::unique_ptr<Base::View>(AssetManager*)>;
public:
	std::unordered_map<std::type_index, FabricCallback> ViewFabrics;
	std::stack<std::unique_ptr<Base::View>> ViewStack;
	std::type_index MainView;
private:
	AssetManager* m_GlobalViewAssetMgr;
public:
	ViewManager();
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
		FabricCallback fabric = ([](AssetManager* assetMgr) -> std::unique_ptr<Base::View> { return std::make_unique<TView>(assetMgr); });

		Logger::GetInstance()->AddInfo(std::format("Registing {}", typeid(TView).name()), typeid(ViewManager));

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
};