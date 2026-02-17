#include "BeatEngine/Manager/ViewManager.h"

template<typename TView>
    requires(std::is_base_of_v<Base::View, TView>)
void ViewManager::Push() {
    Push(std::type_index(typeid(TView)));
}

template<typename TView>
    requires(std::is_base_of_v<Base::View, TView>)
void ViewManager::RegisterView() {
    auto ID = std::type_index(typeid(TView));
    FabricCallback fabric = ([](GameContext* context, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr)
        -> std::unique_ptr<Base::View> { return std::make_unique<TView>(context, assetMgr, settingsMgr, audioMgr, uiMgr); });

    Logger::AddInfo(typeid(ViewManager), "Registing {}", typeid(TView).name());

    bool firstView = ViewFabrics.empty();

    if (ViewFabrics.find(ID) == ViewFabrics.end()) {
        ViewFabrics.try_emplace(ID, fabric);
        if (firstView)
            MainView = ID;
    }
}
