#pragma once

#include <memory>

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/UI/Elements/UIPanel.h"
#include "BeatEngine/UI/UIElement.h"
#include "BeatEngine/UI/Elements/UIPanel.h"

class UILayer /*: public sf::Drawable*/ {
private:
	std::shared_ptr<UIElement> m_Root = nullptr;
	std::shared_ptr<UIPanel> m_BackPanel = nullptr;

	bool m_Hidden = false;

	Vector2f m_Size = { 0, 0 };
	Vector2f m_Position = { 0, 0 };
public:
	UILayer() = default;
	UILayer(Vector2f size, Vector2f position);

	template <typename TUI> 
		requires(std::is_base_of_v<UIElement, TUI>)
	std::shared_ptr<TUI> SetRootElement();

	template <typename TUI>
		requires(std::is_base_of_v<UIElement, TUI>)
	std::shared_ptr<TUI> GetRootElement();

	void SetLayerBackPanel();
	void SetLayerBackPanel(std::shared_ptr<UIPanel> backPanel);
	void OnEvent(std::optional<Base::Event> event);
	
	void Update(float dt);
	void Render(/*sf::RenderTarget& target, sf::RenderStates states */) const /*override*/;

    void SetVisible(bool visible);

	bool IsVisible() const { return !m_Hidden; }
};	

#include "BeatEngine/UI/UILayer.inl"
