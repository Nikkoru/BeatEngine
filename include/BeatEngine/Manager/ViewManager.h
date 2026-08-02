#pragma once

#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Util/Optional.hpp"
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
	~ViewManager() { SignalManager::GetInstance()->RemoveCallbacks(typeid(ViewManager)); };
public:
    void SetContext(GameContext* context) { m_Context = context; }
    void SetState(GameState* state) { m_State = state; }

    void Init();
    void Uninit();
public:
	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	bool Push();
	bool Push(std::type_index viewID);

	void Pop();

	template<typename TView>
		requires(std::is_base_of_v<Base::View, TView>)
	void RegisterView();

	bool OnEvent(Optional<Base::Event> event);
	bool OnDraw();
	bool OnUpdate(float dt);
	bool OnExit();

	bool HasActiveViews();

	void GetViewKeybinds();
public:
    void ShowImGuiDebugWindow();
};

#include "BeatEngine/Manager/ViewManager.inl"
