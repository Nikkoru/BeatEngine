#pragma once

#include "BeatEngine/UI/UIElement.h"

class UIClickeable : public UIElement {
protected:
	bool m_Hovered = false;
protected:
	std::function<void()> OnRClick = nullptr;
	std::function<void()> OnLClick = nullptr;

	std::function<void()> OnHover = nullptr;
	std::function<void()> OnUnHover = nullptr;
public:
	UIClickeable() = default;
	UIClickeable(std::type_index elementID) : UIElement(elementID) {}

	void SetOnRClick(std::function<void()> func);
	void SetOnLClick(std::function<void()> func);
	void SetOnHover(std::function<void()> func);
	void SetOnUnHover(std::function<void()> func);

	void OnMouseMove(sf::Vector2i position);
	void OnMousePressed(sf::Mouse::Button button, sf::Vector2i position);
	void OnMouseReleased(sf::Mouse::Button button, sf::Vector2i position);
};