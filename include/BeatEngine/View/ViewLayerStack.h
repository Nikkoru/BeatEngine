#pragma once

#include "BeatEngine/View/ViewLayer.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Window/Event.hpp>

#include <optional>
#include <map>
#include <typeindex>
#include <memory>

namespace Base {
	class View;
}

class ViewLayerStack : public sf::Drawable {
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

	void OnSFMLEvent(std::optional<sf::Event> event);
	void OnUpdate(float dt);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#include "BeatEngine/View/ViewLayerStack.inl"