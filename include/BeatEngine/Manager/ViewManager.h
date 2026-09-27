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
class Renderer;
class AppContext;
class AppState;
class ViewManager {
public:
	using FabricCallback = std::function<std::shared_ptr<Base::View>(AppContext*, AppState*)>;
public:
	std::unordered_map<std::type_index, FabricCallback> ViewFabrics;
	std::stack<std::shared_ptr<Base::View>> ViewStack;
	std::type_index MainView;
private:
    AppContext* m_Context{ nullptr };
    AppState* m_State{ nullptr };
public:
    ViewManager() : ViewManager(nullptr, nullptr) {}
	ViewManager(AppContext* context, AppState* state);
	~ViewManager() { SignalManager::GetInstance()->RemoveCallbacks(typeid(ViewManager)); };
public:
    void SetContext(AppContext* context) { m_Context = context; }
    void SetState(AppState* state) { m_State = state; }

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
	bool OnDraw(Renderer* const renderer);
	bool OnUpdate(float dt);
	bool OnExit();

	bool HasActiveViews();

	void GetViewKeybinds();
public:
    void ShowImGuiDebugWindow();
};

#include "BeatEngine/Manager/ViewManager.inl"
