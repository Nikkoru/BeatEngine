#include "BeatEngine/Manager/SystemManager.h"
#include "BeatEngine/AppContext.hpp"
#include "BeatEngine/AppState.hpp"
#include "BeatEngine/Base/System.h"

SystemManager::SystemManager(AppContext* context, AppState* state) 
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
