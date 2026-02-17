#include "BeatEngine/View/ViewLayerStack.h"

#include <ranges>


void ViewLayerStack::AttachLayer(std::shared_ptr<ViewLayer> layer) {
	m_Layers.try_emplace(layer->m_ID, layer);
}

std::shared_ptr<ViewLayer> ViewLayerStack::GetLayer(std::type_index id) {
	if (m_Layers.contains(id)) {
		return m_Layers.at(id);
	}

	return nullptr;
}

void ViewLayerStack::OnEvent(std::optional<Base::Event> event) {
	for (const auto& [type, layer] : m_Layers) {
		layer->OnEvent(event);
	}
}

void ViewLayerStack::OnUpdate(float dt) {
	for (const auto& [type, layer] : m_Layers) {
		layer->OnUpdate(dt);
	}
}

void ViewLayerStack::draw(/*sf::RenderTarget& target, sf::RenderStates states*/) const {
	// for (const auto& [type, layer] : m_Layers) {
	// 	layer->draw(target, states);
	// }
}
