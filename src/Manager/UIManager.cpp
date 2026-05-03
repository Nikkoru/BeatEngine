#include "BeatEngine/Manager/UIManager.h"

#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/UI/UILayer.h"
#include "BeatEngine/Logger.h"
#include <memory>

UIManager::UIManager(GameContext* context, GameState* state)
    : m_Context(context), m_State(state) {}

void UIManager::OnEvent(std::optional<Base::Event> event) {
	for (const auto& [name, layer] : m_GlobalLayers) {
		layer->OnEvent(event);
	}

	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
		layer->OnEvent(event);
	}
}

std::shared_ptr<UILayer> UIManager::AddLayer(const std::string layerName, bool global) {
	auto layer = std::make_shared<UILayer>();
	
	if (global) {
        if (m_GlobalLayers.contains(layerName)) {
            Logger::AddWarning(typeid(UIManager), "Layer named \"{}\" already exists, returning existing one", layerName);
            layer = m_GlobalLayers.at(layerName);
        }
        else 
		    m_GlobalLayers[layerName] = layer;
    }
	else {
        if (!m_Layers.contains(m_Context->ActiveView)) {
            Logger::AddDebug(typeid(UIManager), "View \"{}\" doesn't have a entry. Creating and adding layer \"{}\"", m_Context->ActiveView.name(), layerName);
            m_Layers[m_Context->ActiveView].try_emplace(layerName, layer);
        }
        else {
            if (!m_Layers.at(m_Context->ActiveView).contains(layerName)) {
                m_Layers.at(m_Context->ActiveView)[layerName] = layer;
            }
            else {
                Logger::AddWarning(typeid(UIManager), "Layer named \"{}\" already exists, returning existing one", layerName);
                layer = m_Layers.at(m_Context->ActiveView)[layerName];
            }
        }
    }

	return layer;
}

void UIManager::RemoveLayer(const std::string layerName, bool global) {
	m_Layers[m_Context->ActiveView].erase(layerName);
}

void UIManager::RemoveViewLayers(const std::type_index viewID) {
	m_Layers.erase(viewID);
}

void UIManager::RemoveGlobalLayers() {
	m_GlobalLayers.clear();
}

void UIManager::RemoveAllLayers() {
	m_Layers.clear();
	m_GlobalLayers.clear();
}

void UIManager::OnDraw() {
	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
		// window->Render(*layer);
        
	}
	for (const auto& [name, layer] : m_GlobalLayers) {
		// window->Render(*layer);
	}
}

void UIManager::Update(float dt) {
	for (const auto& [name, layer] : m_GlobalLayers) {
		layer->Update(dt);
	}
	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
		layer->Update(dt);
	}
}

void UIManager::ShowImGuiDebugWindow() {

}
