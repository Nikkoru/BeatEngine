#include "BeatEngine/Manager/ViewManager.h"

#include <format>

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Base/Events/ViewEvent.h"
#include "BeatEngine/Logger.h"

ViewManager::ViewManager() : MainView(typeid(nullptr)) {
	
}

void ViewManager::PopView() {
	if (!ViewStack.size() > 1) {
		ViewStack.pop();
		Logger::GetInstance()->AddInfo("Popped top view!", typeid(ViewManager));
	}
	else
		Logger::GetInstance()->AddInfo("Only one or no view. Not popping view", typeid(ViewManager));
}

void ViewManager::OnSFMLEvent(std::optional<sf::Event> event) {
	ViewStack.top()->OnSFMLEvent(event);
}

void ViewManager::OnDraw(sf::RenderWindow* window) {
	ViewStack.top()->OnDraw();
}

void ViewManager::OnUpdate() {
	ViewStack.top()->OnUpdate();
}

void ViewManager::GetViewKeybinds() {
	// placeholder #TOIMPLEMENT
}