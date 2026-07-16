#include "BeatEngine/UI/UIClickeable.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Events/MouseEvents.h"

// #include "BeatEngine/Manager/SignalManager.h"
// #include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Util/Optional.hpp"
#include "BeatEngine/Util/UIHelper.h"
// #include <memory>

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

bool UIClickeable::IsHovered() {
    return this->m_Hovered;
}

void UIClickeable::EventHandler(Optional<Base::Event> event) {
    auto bo = event.HasValue();
	if (!bo)
		return;
	
	if (auto data = event.StaticCastTo<MouseMovedEvent>(); event->Is<MouseMovedEvent>())
		OnMouseMove(data.Position);
	if (auto data = event.StaticCastTo<MouseButtonDownEvent>(); event->Is<MouseButtonDownEvent>())
		OnMousePressed(data.Button, data.Position);
	if (auto data = event.StaticCastTo<MouseButtonUpEvent>(); event->Is<MouseButtonUpEvent>())
		OnMouseReleased(data.Button, data.Position);
}

void UIClickeable::OnMouseMove(Vector2i position) {
	bool currentlyHovered = UIHelper::CheckCollisionRec(position, m_LayoutRect);

	if (currentlyHovered && !m_Hovered) {
        // if (m_CursorFeedback)
        //     SignalManager::GetInstance()->Send(std::make_shared<GameChangeCursorSignal>(sf::Cursor::Type::Hand));
		if (OnHover)
			OnHover();
	}
	else if (!currentlyHovered && m_Hovered) {
        // if (m_CursorFeedback)
        //     SignalManager::GetInstance()->Send(std::make_shared<GameChangeCursorSignal>(sf::Cursor::Type::Arrow));
		if (OnUnHover)
			OnUnHover();
	}
	m_Hovered = currentlyHovered;


	if (m_Active)
		m_Active = currentlyHovered;
}

void UIClickeable::OnMousePressed(Mouse::Button, Vector2i position) {
	bool currentlyHovered = UIHelper::CheckCollisionRec(position, m_LayoutRect);

	if (currentlyHovered) {
		m_Active = true;

		if (OnActive)
			OnActive();
	}

	m_Hovered = currentlyHovered;
}

void UIClickeable::OnMouseReleased(Mouse::Button button, Vector2i position) {
	bool currentlyHovered = UIHelper::CheckCollisionRec(position, m_LayoutRect);

	if (currentlyHovered && m_Active) {
		switch (button) {
		case Mouse::Button::Left:
			if (OnLClick)
				OnLClick();
			break;
		case Mouse::Button::Right:
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
