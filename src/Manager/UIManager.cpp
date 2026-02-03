#include "BeatEngine/Manager/UIManager.h"

#include "BeatEngine/GameContext.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Exception.h"

UIManager::UIManager(GameContext* context) : m_Context(context) {}

void UIManager::OnSFMLEvent(std::optional<sf::Event> event) {
	for (const auto& [name, layer] : m_GlobalLayers) {
		layer->OnSFMLEvent(event);
	}

	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
		layer->OnSFMLEvent(event);
	}
}

std::shared_ptr<UILayer> UIManager::AddLayer(const std::string layerName, bool global) {
	auto layer = std::make_shared<UILayer>();
	
	if (global) {
        if (m_GlobalLayers.contains(layerName)) {
            Logger::AddError(typeid(UIManager), "Layer named \"{}\" already exists", layerName);
            return nullptr;
        }
        else 
		    m_GlobalLayers[layerName] = layer;
    }
	else {
        if (!m_Layers.contains(m_Context->ActiveView)) {
            Logger::AddInfo(typeid(UIManager), "View \"{}\" doesn't have a entry. Creating and adding layer \"{}\"", m_Context->ActiveView.name(), layerName);
            m_Layers[m_Context->ActiveView].try_emplace(layerName, layer);
        }
        else {
            if (m_Layers.at(m_Context->ActiveView).contains(layerName)) {
                Logger::AddError(typeid(UIManager), "Layer named \"{}\" already exists", layerName);
                return nullptr;
            }
            else 
                m_Layers.at(m_Context->ActiveView).at(layerName) = layer;
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

void UIManager::OnDraw(sf::RenderWindow* window) {
	for (const auto& [name, layer] : m_Layers[m_Context->ActiveView]) {
		window->draw(*layer);
	}
	for (const auto& [name, layer] : m_GlobalLayers) {
		window->draw(*layer);
	}
}

void UIManager::DrawLayer(const std::string layerName, sf::RenderWindow* window) {
	if (m_GlobalLayers.contains(layerName)) {
		window->draw(*m_GlobalLayers[layerName]);
	}
	else if (m_Layers[m_Context->ActiveView].contains(layerName)) {
		window->draw(*m_Layers[m_Context->ActiveView][layerName]);
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

void UIManager::DrawImGuiDebug() {

}
