#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/UI/UILayer.h"

#include <memory>
#include <typeindex>

class UIManager {
private:
	std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<UILayer>>> m_Layers;
	std::unordered_map<std::string, std::shared_ptr<UILayer>> m_GlobalLayers;
private:
    std::shared_ptr<GameContext> m_Context{ nullptr };
    std::shared_ptr<GameState> m_State{ nullptr };
public:
	UIManager(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state);
	~UIManager() = default;

	void OnEvent(std::optional<Base::Event> event);

	std::shared_ptr<UILayer> AddLayer(const std::string layerName, bool global = false);
	void RemoveLayer(const std::string layerName, bool global = false);

	void RemoveViewLayers(const std::type_index viewID);
	void RemoveGlobalLayers();
	void RemoveAllLayers();

	void OnDraw(GraphicsManager* window);

	void Update(float dt);
    
    void DrawImGuiDebug();
};
