#include "BeatEngine/Manager/ViewManager.h"

#include <format>

#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/ViewSignals.h"

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Events/ViewEvent.h"
#include "BeatEngine/Logger.h"

ViewManager::ViewManager() : MainView(typeid(nullptr)) {
	SignalManager::GetInstance()->RegisterCallback<ViewPushSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		auto sigView = std::static_pointer_cast<ViewPushSignal>(sig);
		Push(sigView->ViewID);
	});
	SignalManager::GetInstance()->RegisterCallback<ViewPopSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		Pop();
	});
	SignalManager::GetInstance()->RegisterCallback<ViewSuspendSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		ViewStack.top()->OnSuspend();
	});
	SignalManager::GetInstance()->RegisterCallback<ViewUnsuspendSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		ViewStack.top()->OnResume();
	});
}

void ViewManager::Push(std::type_index viewID) {
	if (!ViewFabrics.empty()) {
		if (ViewFabrics.contains(viewID)) {
			ViewStack.push(ViewFabrics[viewID](m_GlobalViewAssetMgr, m_GlobalViewSettingsMgr, m_GlobalViewAudioMgr, m_GlobalViewUIMgr));
			MainView = ViewStack.top()->b_ID;

			Logger::GetInstance()->AddInfo(std::format("{} pushed!", viewID.name()), typeid(ViewManager));

			EventManager::GetInstance()->UpdateMainView(MainView);
			EventManager::GetInstance()->Send(std::make_shared<ViewPushEvent>(MainView));
		}
		else
			Logger::GetInstance()->AddError("View not registed. You need to register the view with RegisterView<TView>()", typeid(ViewManager));
	}
	else
		Logger::GetInstance()->AddError("No fabrics found", typeid(ViewManager));

}

void ViewManager::Pop() {
	if (ViewStack.size() > 1) {
		ViewStack.pop();
		MainView = ViewStack.top()->b_ID;

		Logger::GetInstance()->AddInfo("Popped top view!", typeid(ViewManager));

		EventManager::GetInstance()->UpdateMainView(MainView);
		EventManager::GetInstance()->Send(std::make_shared<ViewPopEvent>());
	}
	else
		Logger::GetInstance()->AddInfo("Only one or no view. Not popping view", typeid(ViewManager));
}

bool ViewManager::OnSFMLEvent(std::optional<sf::Event> event) {
	if (!ViewStack.empty()) {
		ViewStack.top()->OnSFMLEvent(event);
		return true;
	}
	else {
		Logger::GetInstance()->AddCritical("No view on the stack. Did you forget to push?", typeid(ViewManager));
		return false;
	}
}

bool ViewManager::OnDraw(sf::RenderWindow* window) {
	if (!ViewStack.empty()) {
		ViewStack.top()->OnDraw(window);
		return true;
	}
	else {
		Logger::GetInstance()->AddCritical("No view on the stack. Did you forget to push?", typeid(ViewManager));
		return false;
	}
}

bool ViewManager::OnUpdate(float dt) {
	if (!ViewStack.empty()) {
		ViewStack.top()->OnUpdate(dt);
		return true;
	}
	else {
		Logger::GetInstance()->AddCritical("No view on the stack. Did you forget to push?", typeid(ViewManager));
		return false;
	}
}

bool ViewManager::OnExit() {
	if (!ViewStack.empty()) {
		ViewStack.top()->OnExit();
		return true;
	}

	return false;
}

bool ViewManager::HasActiveViews() {
	return !ViewStack.empty();
}

void ViewManager::GetViewKeybinds() {
	// placeholder #TOIMPLEMENT
}

void ViewManager::SetGlobalAssetManager(AssetManager* assetMgr) {
	this->m_GlobalViewAssetMgr = assetMgr;
}
