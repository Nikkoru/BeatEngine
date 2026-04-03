#pragma once

#include <typeindex>
#include <map>
#include <memory>

namespace Base {
    class System;
};
class GameContext;
class GameState;
class SystemManager {
private:
	std::map<std::type_index, std::shared_ptr<Base::System>> m_Systems;
private:
    GameContext* m_Context{ nullptr };
    GameState* m_State{ nullptr };
public:
    SystemManager() : SystemManager(nullptr, nullptr) {}
	SystemManager(GameContext* context, GameState* state);
	~SystemManager() = default;
public:
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
