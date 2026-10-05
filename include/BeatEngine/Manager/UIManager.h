#pragma once

#include "BeatEngine/UI/UIElement.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace Base {
    class Event;
};
class UILayer;
class AppContext;
class AppState;
class UIManager {
private:
	std::unordered_map<std::type_index, std::unordered_map<std::string, std::unique_ptr<UILayer>>> m_Layers;
	std::unordered_map<std::string, std::unique_ptr<UILayer>> m_GlobalLayers;
private:
    AppContext* m_Context{ nullptr };
    AppState* m_State{ nullptr };
public:
    UIManager() : UIManager(nullptr, nullptr) {}
	UIManager(AppContext* context, AppState* state);
	~UIManager();

    void SetContext(AppContext* context) { m_Context = context; }
    void SetState(AppState* state) { m_State = state; }

	void OnEvent(Optional<Base::Event> event);

	UILayer* AddLayer(const std::string layerName, bool global = false);
	void RemoveLayer(const std::string layerName, bool global = false);

	void RemoveViewLayers(const std::type_index viewID);
	void RemoveGlobalLayers();
	void RemoveAllLayers();

	void OnDraw(Renderer* const renderer);

	void Update(float dt);
    
    void ShowImGuiDebugWindow();

    void Uninit();
private:
    void DrawDebugElement(UIElement& element);
};
