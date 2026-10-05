#pragma once

#include <memory>

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/Vector2.h"

class Renderer;
class UIElement;
class UIPanel;
class UILayer : public GraphicalElement {
private:
	std::unique_ptr<UIElement> m_Root{ nullptr };
	std::unique_ptr<UIPanel> m_BackPanel{ nullptr };

	bool m_Hidden{ false };

	Vector2f m_Size{ 0, 0 };
	Vector2f m_Position{ 0, 0 };
public:
	UILayer() = default;
	UILayer(Vector2f size, Vector2f position);
    ~UILayer() override;

	template <typename TUI> 
		requires(std::is_base_of_v<UIElement, TUI>)
	TUI* SetRootElement();

	template <typename TUI>
		requires(std::is_base_of_v<UIElement, TUI>)
	TUI* GetRootElement();

	void SetLayerBackPanel();
	void OnEvent(Optional<Base::Event> event);
	
	void Update(float dt);
	void Draw(Renderer* const mgr, RenderState state = RenderState::Default) override;
    void UninitGraphics(Renderer* const mgr) override;

    void SetVisible(bool visible);

	bool IsVisible() const { return !m_Hidden; }
};	

#include "BeatEngine/UI/UILayer.inl"
