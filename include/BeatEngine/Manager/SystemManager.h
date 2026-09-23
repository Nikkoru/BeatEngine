#pragma once

#include <typeindex>
#include <map>
#include <memory>

namespace Base {
    class System;
};
class AppContext;
class AppState;
class SystemManager {
private:
	std::map<std::type_index, std::shared_ptr<Base::System>> m_Systems;
private:
    AppContext* m_Context{ nullptr };
    AppState* m_State{ nullptr };
public:
    SystemManager() : SystemManager(nullptr, nullptr) {}
	SystemManager(AppContext* context, AppState* state);
	~SystemManager() = default;
public:
    void SetContext(AppContext* context) { m_Context = context; }
    void SetState(AppState* state) { m_State = state; }

	template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void RegisterSystem();

	template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void StartSystem();

    template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void StopSystem(); 

	void StartSystems();
	void StopSystems();

	void Update(float dt);

    void DrawImGuiDebug();
};

#include "BeatEngine/Manager/SystemManager.inl"
