#include "BeatEngine/Manager/ViewManager.h"

#include "BeatEngine/Logger.h"
#include "BeatEngine/Base/View.h"

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
    FabricCallback fabric = ([](GameContext* context, GameState* state)
        -> std::shared_ptr<Base::View> { return std::make_shared<TView>(context, state); });

    Logger::AddInfo(typeid(ViewManager), "Registing {}", typeid(TView).name());

    bool firstView = ViewFabrics.empty();

    if (ViewFabrics.find(ID) == ViewFabrics.end()) {
        ViewFabrics.try_emplace(ID, fabric);
        if (firstView)
            MainView = ID;
    }
}
