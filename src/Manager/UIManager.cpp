#include "BeatEngine/Manager/UIManager.h"

#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Exception.h"

void UIManager::OnSFMLEvent(std::optional<sf::Event> event) {
	for (const auto& [name, layer] : m_GlobalLayers) {
		layer->OnSFMLEvent(event);
	}

	for (const auto& [name, layer] : m_Layers[m_CurrentView]) {
		layer->OnSFMLEvent(event);
	}
}

std::shared_ptr<UILayer> UIManager::AddLayer(const std::string layerName, bool global) {
	auto layer = std::make_shared<UILayer>();
	
	if (global)
		m_GlobalLayers[layerName] = layer;
	else {
		if (m_CurrentView != typeid(nullptr))
			m_Layers[m_CurrentView][layerName] = layer;
		else {
			std::string msg = "No view is present, cannot add layer";

			Logger::GetInstance()->AddCritical(msg, typeid(UIManager));
			THROW_RUNTIME_ERROR(msg);
		}
	}

	return layer;
}

void UIManager::RemoveLayer(const std::string layerName, bool global) {
	m_Layers[m_CurrentView].erase(layerName);
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
	for (const auto& [name, layer] : m_GlobalLayers) {
		window->draw(*layer);
	}
	for (const auto& [name, layer] : m_Layers[m_CurrentView]) {
		window->draw(*layer);
	}
}

void UIManager::DrawLayer(const std::string layerName, sf::RenderWindow* window) {
	if (m_GlobalLayers.contains(layerName)) {
		window->draw(*m_GlobalLayers[layerName]);
	}
	else if (m_Layers[m_CurrentView].contains(layerName)) {
		window->draw(*m_Layers[m_CurrentView][layerName]);
	}
}

void UIManager::Update(float dt) {
	for (const auto& [name, layer] : m_GlobalLayers) {
		layer->Update(dt);
	}
	for (const auto& [name, layer] : m_Layers[m_CurrentView]) {
		layer->Update(dt);
	}
}

void UIManager::UpdateView(const std::type_index& viewID) {
	m_CurrentView = viewID;
}
