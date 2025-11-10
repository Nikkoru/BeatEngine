#pragma once

#include "BeatEngine/Base/View.h"

#include <functional>
#include <memory>
#include <typeindex>
#include <stack>
#include <unordered_map>

class ViewManager {
public:
	using FabricCallback = std::function<std::unique_ptr<Base::View>()>;
public:
	std::unordered_map<std::type_index, FabricCallback> ViewFabrics;
	std::stack<std::unique_ptr<Base::View>> ViewStack;
	std::type_index MainView;
public:
	ViewManager();
	~ViewManager() = default;
public:
	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void PushView();
	void PopView();

	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void RegisterView();

	void OnSFMLEvent(std::optional<sf::Event> event);
	void OnDraw(sf::RenderWindow* window);
	void OnUpdate();

	void GetViewKeybinds();
};