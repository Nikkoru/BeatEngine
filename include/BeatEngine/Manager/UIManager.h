#pragma once

#include "BeatEngine/GameContext.h"
#include "BeatEngine/UI/UILayer.h"

#include <typeindex>

class UIManager {
private:
	std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<UILayer>>> m_Layers;
	std::unordered_map<std::string, std::shared_ptr<UILayer>> m_GlobalLayers;
private:
    GameContext* m_Context = nullptr;
public:
	UIManager(GameContext* context);
	~UIManager() = default;

	void OnSFMLEvent(std::optional<sf::Event> event);

	std::shared_ptr<UILayer> AddLayer(const std::string layerName, bool global = false);
	void RemoveLayer(const std::string layerName, bool global = false);

	void RemoveViewLayers(const std::type_index viewID);
	void RemoveGlobalLayers();
	void RemoveAllLayers();

	void OnDraw(sf::RenderWindow* window);
	void DrawLayer(const std::string layerName, sf::RenderWindow* window);

	void Update(float dt);
    
    void DrawImGuiDebug();
};
