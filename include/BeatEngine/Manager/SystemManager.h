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
			Logger::GetInstance()->AddInfo("System already registered: " + std::string(typeid(TSystem).name()), typeid(SystemManager));
		else {
			auto system = std::make_shared<TSystem>();
			m_Systems.try_emplace(ID, system);
			Logger::GetInstance()->AddInfo("Registered system: " + std::string(typeid(TSystem).name()), typeid(SystemManager));
		}
	}

	template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void StartSystem() {
		std::type_index ID = typeid(TSystem);

		if (m_Systems.contains(ID))
			m_Systems.at(ID)->Start();
		else
			Logger::GetInstance()->AddWarning("System " + std::string(typeid(TSystem).name()) + " not found", typeid(SystemManager));
	}
	template <typename TSystem>
		requires(std::is_base_of_v<Base::System, TSystem>)
	void StopSystem() {
		std::type_index ID = typeid(TSystem);

		if (m_Systems.contains(ID))
			m_Systems.at(ID)->Stop();
		else
			Logger::GetInstance()->AddWarning("System " + std::string(typeid(TSystem).name()) + " not found", typeid(SystemManager));
	}

	void StartSystems();
	void StopSystems();

	void Update(float dt);
};