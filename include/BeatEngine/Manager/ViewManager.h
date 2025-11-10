#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/Logger.h"

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
	void PushView() {
		if (!ViewStack.empty()) {
			auto ID = std::type_index(typeid(TView));

			if (ViewFabrics.contains(ID)) {
				ViewStack.push(ViewFabrics[ID]());
				MainView = ViewStack.top()->ID;

				Logger::GetInstance()->AddInfo(std::format("{} pushed!", typeid(TView).raw_name()), typeid(ViewManager));
			}
			else
				Logger::GetInstance()->AddError("View not registed. You need to register the view with RegisterView<TView>()", typeid(ViewManager));
		}
		else
			Logger::GetInstance()->AddError("No fabrics found", typeid(ViewManager));
	}
	void PopView();

	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void RegisterView() {
		auto ID = std::type_index(typeid(TView));
		auto fabric = ([ID]() -> std::unique_ptr<Base::View> { return std::make_unique<TView>(ID); });

		Logger::GetInstance()->AddInfo(std::format("Registing {}", typeid(TView).raw_name()), typeid(ViewManager));
	}

	void OnSFMLEvent(std::optional<sf::Event> event);
	void OnDraw(sf::RenderWindow* window);
	void OnUpdate();

	void GetViewKeybinds();
};