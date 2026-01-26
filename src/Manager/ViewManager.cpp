#include "BeatEngine/Manager/ViewManager.h"

#include <format>
#include <memory>

#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Manager/AudioManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Signals/ViewSignals.h"

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Events/ViewEvent.h"
#include "BeatEngine/Logger.h"

ViewManager::ViewManager(GameContext* context) : MainView(typeid(nullptr)), m_Context(context) {
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
    
    EventManager::GetInstance()->Subscribe<GameExitingEvent>([this](const std::shared_ptr<Base::Event> event) {
        while (!ViewStack.empty()) {
            ViewStack.top()->OnExit();

            ViewStack.pop();
        }    
    }); 
}

void ViewManager::Push(std::type_index viewID) {
	if (!ViewFabrics.empty()) {
		if (ViewFabrics.contains(viewID)) {
			ViewStack.push(ViewFabrics[viewID](m_Context, m_GlobalViewAssetMgr, m_GlobalViewSettingsMgr, m_GlobalViewAudioMgr, m_GlobalViewUIMgr));
			MainView = ViewStack.top()->b_ID;

			Logger::AddInfo(typeid(ViewManager), "{} pushed!", viewID.name());

			EventManager::GetInstance()->UpdateMainView(MainView);
			EventManager::GetInstance()->Send(std::make_shared<ViewPushEvent>(MainView));
		}
		else
			Logger::AddError(typeid(ViewManager), "View not registed. You need to register the view with RegisterView<TView>()");
	}
	else
		Logger::AddError(typeid(ViewManager), "No fabrics found");

}

void ViewManager::Pop() {
	if (ViewStack.size() > 1) {
		ViewStack.pop();
		MainView = ViewStack.top()->b_ID;

		Logger::AddInfo(typeid(ViewManager), "Popped top view!");

		EventManager::GetInstance()->UpdateMainView(MainView);
		EventManager::GetInstance()->Send(std::make_shared<ViewPopEvent>(typeid(MainView)));
	}
	else
		Logger::AddInfo(typeid(ViewManager), "Only one or no view. Not popping view");
}

bool ViewManager::OnSFMLEvent(std::optional<sf::Event> event) {
	if (!ViewStack.empty()) {
        if (!((event->is<sf::Event::KeyPressed>() || event->is<sf::Event::KeyReleased>()) && m_Context->VFlags & ViewFlags_DisableKeys))
		    ViewStack.top()->OnSFMLEvent(event);
		return true;
	}
	else {
		Logger::AddCritical(typeid(ViewManager), "No view on the stack. Did you forgot to push?");
		return false;
	}
}

bool ViewManager::OnDraw(sf::RenderWindow* window) {
	if (!ViewStack.empty()) {
		ViewStack.top()->OnDraw(window);
		return true;
	}
	else {
		Logger::AddCritical(typeid(ViewManager), "No view on the stack. Did you forgot to push?");
		return false;
	}
}

bool ViewManager::OnUpdate(float dt) {
	if (!ViewStack.empty()) {
		ViewStack.top()->OnUpdate(dt);
		return true;
	}
	else {
		Logger::AddCritical(typeid(ViewManager), "No view on the stack. Did you forgot to push?");
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

void ViewManager::SetGlobalAudioManager(AudioManager* audioMgr) {
    this->m_GlobalViewAudioMgr = audioMgr;
}

void ViewManager::SetGlobalSettingsManager(SettingsManager* settingsMgr) {
    this->m_GlobalViewSettingsMgr = settingsMgr;
}

void ViewManager::SetGlobalUIManager(UIManager* uiMgr) {
    this->m_GlobalViewUIMgr = uiMgr;
}
