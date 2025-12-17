#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

#include "BeatEngine/UI/Elements/UIPanel.h"
#include "BeatEngine/UI/UIElement.h"
<<<<<<< HEAD
#include "BeatEngine/UI/Elements/UIPanel.h"
=======

>>>>>>> c315531 (partial initial changes to UI alignment & fullscreen base setting)

class UILayer : public sf::Drawable {
private:
	std::shared_ptr<UIElement> m_Root = nullptr;
	std::shared_ptr<UIPanel> m_BackPanel = nullptr;

	bool m_Hidden = false;

	sf::Vector2f m_Size = { 0, 0 };
	sf::Vector2f m_Position = { 0, 0 };
public:
	UILayer() = default;
	UILayer(sf::Vector2f size, sf::Vector2f position);

	template <typename TUI> 
		requires(std::is_base_of_v<UIElement, TUI>)
	std::shared_ptr<TUI> SetRootElement();

	template <typename TUI>
		requires(std::is_base_of_v<UIElement, TUI>)
	std::shared_ptr<TUI> GetRootElement();

	void SetLayerBackPanel();
	void SetLayerBackPanel(std::shared_ptr<UIPanel> backPanel);
	void OnSFMLEvent(std::optional<sf::Event> event);
	
	void Update(float dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	bool IsVisible() const { return !m_Hidden; }
};	

#include "BeatEngine/UI/UILayer.inl"
