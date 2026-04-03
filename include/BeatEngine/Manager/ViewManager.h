#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <stack>
#include <unordered_map>


namespace Base {
    class Event;
    class View;
};
class GraphicsManager;
class GameContext;
class GameState;
class ViewManager {
public:
	using FabricCallback = std::function<std::shared_ptr<Base::View>(GameContext*, GameState*)>;
public:
	std::unordered_map<std::type_index, FabricCallback> ViewFabrics;
	std::stack<std::shared_ptr<Base::View>> ViewStack;
	std::type_index MainView;
private:
    GameContext* m_Context{ nullptr };
    GameState* m_State{ nullptr };
public:
    ViewManager() : ViewManager(nullptr, nullptr) {}
	ViewManager(GameContext* context, GameState* state);
	~ViewManager() = default;
public:
	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void Push();
	void Push(std::type_index viewID);

	void Pop();

	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void RegisterView();

	bool OnEvent(std::optional<Base::Event> event);
	bool OnDraw(GraphicsManager* window);
	bool OnUpdate(float dt);
	bool OnExit();

	bool HasActiveViews();

	void GetViewKeybinds();
public:
    void ShowImGuiDebugData();
};

#include "BeatEngine/Manager/ViewManager.inl"
