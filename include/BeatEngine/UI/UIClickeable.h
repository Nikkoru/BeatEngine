#pragma once

#include "BeatEngine/UI/UIElement.h"

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

	void EventHandler(std::optional<Base::Event> event) override;
private:
	void OnMouseMove(Vector2i position);
	// void OnMousePressed(sf::Mouse::Button button, sf::Vector2i position);
	// void OnMouseReleased(sf::Mouse::Button button, sf::Vector2i position);
};
