#include "BeatEngine/Manager/SystemManager.h"
#include "BeatEngine/GameContext.h"

SystemManager::SystemManager(GameContext* context) : m_Context(context) {}

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

void SystemManager::DrawImGuiDebug() {

}
