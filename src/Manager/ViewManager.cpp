#include "BeatEngine/Manager/ViewManager.h"

#include <format>

#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Base/Signals/ViewSignals.h"

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Base/Events/ViewEvent.h"
#include "BeatEngine/Logger.h"

ViewManager::ViewManager() : MainView(typeid(nullptr)) {
	SignalManager::GetInstance()->RegisterCallback<ViewPushSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		auto sigView = std::static_pointer_cast<ViewPushSignal>(sig);
		PushView(sigView->ViewID);
	});
	SignalManager::GetInstance()->RegisterCallback<ViewPopSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		PopView();
	});
	SignalManager::GetInstance()->RegisterCallback<ViewSuspendSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		ViewStack.top()->OnSuspend();
	});
	SignalManager::GetInstance()->RegisterCallback<ViewUnsuspendSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		ViewStack.top()->OnResume();
	});
}

void ViewManager::PushView(std::type_index viewID) {
	if (!ViewStack.empty()) {

		if (ViewFabrics.contains(viewID)) {
			ViewStack.push(ViewFabrics[viewID]());
			MainView = ViewStack.top()->ID;

			Logger::GetInstance()->AddInfo(std::format("{} pushed!", viewID.name()), typeid(ViewManager));
			
			EventManager::GetInstance()->Send(std::make_shared<ViewPushEvent>(MainView));
		}
		else
			Logger::GetInstance()->AddError("View not registed. You need to register the view with RegisterView<TView>()", typeid(ViewManager));
	}
	else
		Logger::GetInstance()->AddError("No fabrics found", typeid(ViewManager));

}

void ViewManager::PopView() {
	if (ViewStack.size() > 1) {
		ViewStack.pop();
		MainView = ViewStack.top()->ID;

		Logger::GetInstance()->AddInfo("Popped top view!", typeid(ViewManager));

		EventManager::GetInstance()->Send(std::make_shared<ViewPopEvent>(MainView));
	}
	else
		Logger::GetInstance()->AddInfo("Only one or no view. Not popping view", typeid(ViewManager));
}

void ViewManager::OnSFMLEvent(std::optional<sf::Event> event) {
	ViewStack.top()->OnSFMLEvent(event);
}

void ViewManager::OnDraw(sf::RenderWindow* window) {
	ViewStack.top()->OnDraw(window);
}

void ViewManager::OnUpdate() {
	ViewStack.top()->OnUpdate();
}

void ViewManager::GetViewKeybinds() {
	// placeholder #TOIMPLEMENT
}