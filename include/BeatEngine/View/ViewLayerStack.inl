#include "BeatEngine/View/ViewLayerStack.h"

template<typename TLayer>
	requires(std::is_base_of_v<ViewLayer, TLayer>)
inline std::shared_ptr<TLayer> ViewLayerStack::AttachLayer() {
	auto& id = typeid(TLayer);
	if (!m_Layers.contains(id)) {
		m_Layers.try_emplace(id, std::make_shared<TLayer>());
		m_LayerCount++;
	}

	auto& layer = m_Layers.at(id);
	layer->SetLayerIndex(m_LayerCount);

	return layer;
}