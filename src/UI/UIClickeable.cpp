#include "BeatEngine/UI/UIClickeable.h"

#include "BeatEngine/Util/UIHelper.h"

void UIClickeable::SetOnRClick(std::function<void()> func) {
	this->OnRClick = func;
}

void UIClickeable::SetOnLClick(std::function<void()> func) {
	this->OnLClick = func;
}

void UIClickeable::SetOnMClick(std::function<void()> func) {
	this->OnMClick = func;
}

void UIClickeable::SetOnHover(std::function<void()> func) {
	this->OnHover = func;
}

void UIClickeable::SetOnUnHover(std::function<void()> func) {
	this->OnUnHover = func;
}

void UIClickeable::EventHandler(std::optional<sf::Event> event) {
	if (!event.has_value())
		return;
	
	if (auto data = event->getIf<sf::Event::MouseMoved>())
		OnMouseMove(data->position);
	if (auto data = event->getIf<sf::Event::MouseButtonPressed>())
		OnMousePressed(data->button, data->position);
	if (auto data = event->getIf<sf::Event::MouseButtonReleased>())
		OnMouseReleased(data->button, data->position);
}

void UIClickeable::OnMouseMove(sf::Vector2i position) {
	bool currentlyHovered = UIHelper::CheckCollisionRec(position, m_LayoutRect);

	if (currentlyHovered && !m_Hovered) {
		if (OnHover)
			OnHover();
	}
	else if (!currentlyHovered && m_Hovered) {
		if (OnUnHover)
			OnUnHover();
	}
	m_Hovered = currentlyHovered;

	if (m_Active)
		m_Active = currentlyHovered;
}

void UIClickeable::OnMousePressed(sf::Mouse::Button button, sf::Vector2i position) {
	bool currentlyHovered = UIHelper::CheckCollisionRec(position, m_LayoutRect);

	if (currentlyHovered) {
		m_Active = true;

		if (OnActive)
			OnActive();
	}

	m_Hovered = currentlyHovered;
}

void UIClickeable::OnMouseReleased(sf::Mouse::Button button, sf::Vector2i position) {
	bool currentlyHovered = UIHelper::CheckCollisionRec(position, m_LayoutRect);

	if (currentlyHovered && m_Active) {
		switch (button) {
		case sf::Mouse::Button::Left:
			if (OnLClick)
				OnLClick();
			break;
		case sf::Mouse::Button::Right:
			if (OnRClick)
				OnRClick();
			break;
		}

		if (OnDeactive)
			OnDeactive();
	}
	else {
		if (OnUnHover)
			OnUnHover();
		m_Hovered = false;
	}
	m_Active = false;
}