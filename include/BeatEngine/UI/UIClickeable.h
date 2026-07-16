#pragma once

#include "BeatEngine/UI/UIElement.h"
#include "BeatEngine/Util/Optional.hpp"
#include "BeatEngine/Windows/Mouse.hpp"

class UIClickeable : public UIElement {
protected:
	bool m_Hovered = false;
    bool m_CursorFeedback = true;
protected:
	std::function<void()> OnRClick = nullptr;
	std::function<void()> OnLClick = nullptr;
	std::function<void()> OnMClick = nullptr;

	std::function<void()> OnHover = nullptr;
	std::function<void()> OnUnHover = nullptr;

public:
	UIClickeable() = default;
	UIClickeable(std::type_index elementID) : UIElement(elementID) {}

	void SetOnRClick(std::function<void()> func);
	void SetOnLClick(std::function<void()> func);
	void SetOnMClick(std::function<void()> func);
	void SetOnHover(std::function<void()> func);
	void SetOnUnHover(std::function<void()> func);

    bool IsHovered();

	void EventHandler(Optional<Base::Event> event) override;
private:
	void OnMouseMove(Vector2i position);
	void OnMousePressed(Mouse::Button button, Vector2i position);
	void OnMouseReleased(Mouse::Button button, Vector2i position);
};
