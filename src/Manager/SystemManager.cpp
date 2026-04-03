#include "BeatEngine/Manager/SystemManager.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Base/System.h"

SystemManager::SystemManager(GameContext* context, GameState* state) 
    : m_Context(context), m_State(state) {}

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
