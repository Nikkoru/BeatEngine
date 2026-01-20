#pragma once

#include <typeindex>
#include <memory>

#include "BeatEngine/Base/System.h"
#include "BeatEngine/Logger.h"

class SystemManager {
private:
	std::map<std::type_index, std::shared_ptr<Base::System>> m_Systems;
public:
	SystemManager() = default;
	~SystemManager() = default;
public:
	template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void RegisterSystem() {
		std::type_index ID = typeid(TSystem);

		if (m_Systems.contains(ID))
			Logger::AddInfo(typeid(SystemManager), "System \"{}\" already registered", typeid(TSystem).name());
		else {
			auto system = std::make_shared<TSystem>();
			m_Systems.try_emplace(ID, system);
			Logger::AddInfo(typeid(SystemManager), "Registered system \"{}\"", typeid(TSystem).name());
		}
	}

	template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void StartSystem() {
		std::type_index ID = typeid(TSystem);

		if (m_Systems.contains(ID))
			m_Systems.at(ID)->Start();
		else
			Logger::AddWarning(typeid(SystemManager), "System \"{}\" not found", typeid(TSystem).name());
	}
	template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void StopSystem() {
		std::type_index ID = typeid(TSystem);

		if (m_Systems.contains(ID))
			m_Systems.at(ID)->Stop();
		else
			Logger::AddWarning(typeid(SystemManager), "System \"{}\" not found", typeid(TSystem).name());
	}

	void StartSystems();
	void StopSystems();

	void Update(float dt);

    void DrawImGuiDebug();
};
