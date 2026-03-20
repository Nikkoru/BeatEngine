#include "BeatEngine/GameState.h"
#include "BeatEngine/Manager/ViewManager.h"
#include <memory>

template<typename TView>
    requires(std::is_base_of_v<Base::View, TView>)
void ViewManager::Push() {
    Push(std::type_index(typeid(TView)));
}

template<typename TView>
    requires(std::is_base_of_v<Base::View, TView>)
void ViewManager::RegisterView() {
    auto ID = std::type_index(typeid(TView));
    FabricCallback fabric = ([](std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state)
        -> std::unique_ptr<Base::View> { return std::make_unique<TView>(context, state); });

    Logger::AddInfo(typeid(ViewManager), "Registing {}", typeid(TView).name());

    bool firstView = ViewFabrics.empty();

    if (ViewFabrics.find(ID) == ViewFabrics.end()) {
        ViewFabrics.try_emplace(ID, fabric);
        if (firstView)
            MainView = ID;
    }
}
