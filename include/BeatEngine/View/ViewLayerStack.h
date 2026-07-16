#pragma once

#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/View/ViewLayer.h"

#include <map>
#include <typeindex>
#include <memory>

namespace Base {
	class View;
}

class GraphicsManager;
class ViewLayerStack : public GraphicalElement {
private:
	friend class Base::View;
	std::map<std::type_index, std::shared_ptr<ViewLayer>> m_Layers;

	unsigned int m_LayerCount = 0;
public:
	ViewLayerStack() = default;
	
	template <typename TLayer>
		requires(std::is_base_of_v<ViewLayer, TLayer>)
	std::shared_ptr<TLayer> AttachLayer();
	
	void AttachLayer(std::shared_ptr<ViewLayer> layer);

	std::shared_ptr<ViewLayer> GetLayer(std::type_index id);

	void OnEvent(Optional<Base::Event> event);
	void OnUpdate(float dt);
    void OnDraw(GraphicsManager& mgr);
	void Draw(GraphicsManager& mgr) override;
};

#include "BeatEngine/View/ViewLayerStack.inl"
