#pragma once

#include "BeatEngine/Manager/GraphicsManager.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace Base {
    class Event;
};
class UILayer;
class GameContext;
class GameState;
class UIManager {
private:
	std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<UILayer>>> m_Layers;
	std::unordered_map<std::string, std::shared_ptr<UILayer>> m_GlobalLayers;
private:
    GameContext* m_Context{ nullptr };
    GameState* m_State{ nullptr };
public:
    UIManager() : UIManager(nullptr, nullptr) {}
	UIManager(GameContext* context, GameState* state);
	~UIManager() = default;

	void OnEvent(std::optional<Base::Event> event);

	std::shared_ptr<UILayer> AddLayer(const std::string layerName, bool global = false);
	void RemoveLayer(const std::string layerName, bool global = false);

	void RemoveViewLayers(const std::type_index viewID);
	void RemoveGlobalLayers();
	void RemoveAllLayers();

	void OnDraw();

	void Update(float dt);
    
    void ShowImGuiDebugWindow();
};
