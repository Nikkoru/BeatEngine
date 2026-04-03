#include "BeatEngine/Manager/GraphicsManager.h"

template<typename RendererT>
    requires(std::is_base_of_v<Renderer, RendererT>)
void GraphicsManager::MakeRenderer() {
    MakeRenderer(std::make_shared<RendererT>());
}
