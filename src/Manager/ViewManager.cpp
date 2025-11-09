#include "Manager/ViewManager.h"

#include <format>

#include "Manager/EventManager.h"
#include "Base/Event.h"
#include "Base/Events/ViewEvent.h"
#include "Logger.h"

ViewManager::ViewManager() : MainView(typeid(nullptr)) {
	
}

template<typename TView>
	requires(std::is_base_of_v<Base::View, TView>)
void ViewManager::PushView() {
	if (!ViewStack.empty()) {
		auto ID = std::type_index(typeid(TView));

		if (ViewFabrics.contains(ID)) {
			ViewStack.push(ViewFabrics[ID]());
			MainView = ViewStack.top()->ID;

			Logger::GetInstance()->AddInfo(std::format("{} pushed!", typeid(TView).raw_name()), typeid(ViewManager));
		}
		else
			Logger::GetInstance()->AddError("View not registed. You need to register the view with RegisterView<TView>()", typeid(ViewManager));
	}
	else
		Logger::GetInstance()->AddError("No fabrics found", typeid(ViewManager));
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

template<typename TView>
	requires(std::is_base_of_v<Base::View, TView>)
void ViewManager::RegisterView() {
	auto ID = std::type_index(typeid(TView));
	auto fabric = ([ID]() -> std::unique_ptr<Base::View> { return std::make_unique<TView>(ID); });

	Logger::GetInstance()->AddInfo(std::format("Registing {}", typeid(TView).raw_name()), typeid(ViewManager));
}