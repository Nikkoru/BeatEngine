#include "BeatEngine/Manager/SystemManager.h"

void SystemManager::StartSystems() {
	for (auto& [index, system] : m_Systems) {
		system->Start();
	}
}

void SystemManager::StopSystems() {
	for (auto& [index, system] : m_Systems) {
		system->Stop();
	}
}

void SystemManager::Update(float dt) {
	for (auto& [index, system] : m_Systems) {
		system->Update(dt);
	}
}