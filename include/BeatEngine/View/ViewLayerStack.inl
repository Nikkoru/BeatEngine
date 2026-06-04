#include "BeatEngine/View/ViewLayerStack.h"
#include <memory>

template<typename TLayer>
	requires(std::is_base_of_v<ViewLayer, TLayer>)
inline std::shared_ptr<TLayer> ViewLayerStack::AttachLayer() {
    std::shared_ptr<TLayer> layer = nullptr;

	auto& id = typeid(TLayer);
	if (!m_Layers.contains(id)) {
        layer = std::make_shared<TLayer>();
	}

    AttachLayer(layer);

    return layer;
}
