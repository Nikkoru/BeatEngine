#include "BeatEngine/Game.h"
#include <type_traits>

template<typename TSystem>
    requires(std::is_base_of_v<Base::System, TSystem>)
void Game::RegisterSystem() {
    m_State.GetSystemMgr().RegisterSystem<TSystem>();
}

template<typename TView>
    requires(std::is_base_of_v<Base::View, TView>)
void Game::RegisterView() {
    m_State.GetViewMgr().RegisterView<TView>();
}

template<typename TSettings>
    requires(std::is_base_of_v<Base::Settings, TSettings>)
void Game::RegisterSettings() {
    m_State.GetSettingsMgr().RegisterSettingsData<TSettings>();
}

template<typename TSettings>
    requires(std::is_base_of_v<Base::Settings, TSettings>)
std::shared_ptr<TSettings> Game::GetSettings() {
    auto base = m_State.GetSettingsMgr().GetSettings(typeid(TSettings));

    return std::static_pointer_cast<TSettings>(base);
}

template<typename TLayer>
    requires(std::is_base_of_v<ViewLayer, TLayer>)
void Game::AddGlobalLayer() {
    std::shared_ptr<TLayer> layer = std::make_shared<TLayer>(&m_Context, &m_State);

    m_GlobalLayers.AttachLayer(layer);
}

template<typename TRenderer>
    requires(std::is_base_of_v<Renderer, TRenderer>)
void Game::SetRenderer() {
    m_State.GetGraphicsMgr().MakeRenderer<TRenderer>();
}

template<typename TLayer>
    requires(std::is_base_of_v<ViewLayer, TLayer>)
std::shared_ptr<TLayer> Game::GetGlobalLayer() {
    auto layer = m_GlobalLayers.GetLayer(typeid(TLayer));
    return std::static_pointer_cast<TLayer>(layer);
}
