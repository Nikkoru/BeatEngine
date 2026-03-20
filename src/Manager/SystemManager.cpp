#include "BeatEngine/Manager/SystemManager.h"
#include "BeatEngine/GameContext.h"
#include <memory>

SystemManager::SystemManager(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state) : m_Context(context), m_State(state) {}

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
