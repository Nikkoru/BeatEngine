#include "BeatEngine/View/ViewLayerStack.h"
#include "BeatEngine/Manager/GraphicsManager.h"


void ViewLayerStack::AttachLayer(std::shared_ptr<ViewLayer> layer) {
    m_LayerCount++;
    layer->SetLayerIndex(m_LayerCount);

    layer->Init();
	m_Layers.try_emplace(layer->m_ID, layer);
}

std::shared_ptr<ViewLayer> ViewLayerStack::GetLayer(std::type_index id) {
	if (m_Layers.contains(id)) {
		return m_Layers.at(id);
	}

	return nullptr;
}

void ViewLayerStack::OnEvent(Optional<Base::Event> event) {
	for (const auto& [type, layer] : m_Layers) {
		layer->OnEvent(event);
	}
}

void ViewLayerStack::OnUpdate(float dt) {
	for (const auto& [type, layer] : m_Layers) {
		layer->OnUpdate(dt);
	}
}

void ViewLayerStack::OnDraw(GraphicsManager& mgr, RenderState state) {
    Draw(mgr, state);
}

void ViewLayerStack::Draw(GraphicsManager& mgr, RenderState state) {
	for (const auto& [type, layer] : m_Layers) {
		layer->OnDraw(mgr, state);
	}
}
