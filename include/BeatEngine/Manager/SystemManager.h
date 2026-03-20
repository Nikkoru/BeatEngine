#pragma once

#include <typeindex>
#include <map>
#include <memory>

#include "BeatEngine/Base/System.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"

class SystemManager {
private:
	std::map<std::type_index, std::shared_ptr<Base::System>> m_Systems;
private:
    std::shared_ptr<GameContext> m_Context{ nullptr };
    std::shared_ptr<GameState> m_State{ nullptr };
public:
	SystemManager(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state);
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
