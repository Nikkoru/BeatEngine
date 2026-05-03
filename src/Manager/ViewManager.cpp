#include "BeatEngine/Manager/ViewManager.h"

#include <cstdint>
#include <memory>
#include <typeindex>

#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/ViewSignals.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Base/View.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Events/ViewEvent.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "imgui.h"

ViewManager::ViewManager(GameContext* context, GameState* state) : MainView(typeid(nullptr)), m_Context(context), m_State(state) {
    if (context != nullptr)
        context->ActiveView = MainView;

    SignalManager::GetInstance()->RegisterCallback<ViewPushSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> sig) {
		auto sigView = std::static_pointer_cast<ViewPushSignal>(sig);
		Push(sigView->ViewID);
	});
	SignalManager::GetInstance()->RegisterCallback<ViewPopSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> _) {
		Pop();
	});
	SignalManager::GetInstance()->RegisterCallback<ViewSuspendSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> _) {
		ViewStack.top()->OnSuspend();
	});
	SignalManager::GetInstance()->RegisterCallback<ViewUnsuspendSignal>(typeid(ViewManager), [this](const std::shared_ptr<Base::Signal> _) {
		ViewStack.top()->OnResume();
	});
    
    EventManager::GetInstance()->Subscribe<GameExitingEvent>([](const std::shared_ptr<Base::Event> _) {
    }); 
}

void ViewManager::Init() {

}

void ViewManager::Uninit() {
    while (!ViewStack.empty()) {
        ViewStack.top()->OnExit();
        ViewStack.pop();
    }
}

void ViewManager::Push(std::type_index viewID) {
	if (!ViewFabrics.empty()) {
		if (ViewFabrics.contains(viewID)) {
            if (!ViewStack.empty())
                ViewStack.top()->OnSuspend();
			ViewStack.push(ViewFabrics[viewID](m_Context, m_State));
			MainView = ViewStack.top()->b_ID;

			Logger::AddInfo(typeid(ViewManager), "{} pushed!", viewID.name());
            m_Context->ActiveView = MainView;

			EventManager::GetInstance()->UpdateMainView(MainView);
			EventManager::GetInstance()->Send(std::make_shared<EventViewPush>(MainView));
		}
		else
			Logger::AddError(typeid(ViewManager), "View not registed. You need to register the view with RegisterView<TView>()");
	}
	else
		Logger::AddError(typeid(ViewManager), "No fabrics found");

}

void ViewManager::Pop() {
	if (ViewStack.size() > 1) {
        ViewStack.top()->OnExit();
		ViewStack.pop();
		MainView = ViewStack.top()->b_ID;

		Logger::AddInfo(typeid(ViewManager), "Popped top view!");
        m_Context->ActiveView = MainView;

        ViewStack.top()->OnResume();

		EventManager::GetInstance()->UpdateMainView(MainView);
		EventManager::GetInstance()->Send(std::make_shared<EventViewPop>(typeid(MainView)));
	}
	else
		Logger::AddInfo(typeid(ViewManager), "Only one or no view. Not popping view");
}

bool ViewManager::OnEvent(std::optional<Base::Event> event) {
	if (!ViewStack.empty()) {
		    ViewStack.top()->OnEvent(event);
		return true;
	}
	else {
		Logger::AddCritical(typeid(ViewManager), "No view on the stack. Did you forgot to push?");
		return false;
	}
}

bool ViewManager::OnDraw() {
	if (!ViewStack.empty()) {
		ViewStack.top()->OnDraw();
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
    return true;
}

void ViewManager::GetViewKeybinds() {
	// placeholder #TOIMPLEMENT
}

void ViewManager::ShowImGuiDebugWindow() {
    ImGui::Begin("ViewManager Debug");
    ImGui::Text("Registered Views: %zu", ViewFabrics.size());
    ImGui::Text("Size of stack: %zu", ViewStack.size());
    ImGui::Text("Active View: %s", MainView.name());
    
    uint8_t index{};
    static uint8_t selectedIndex{};
    static std::type_index selectedView = MainView;
    if (ImGui::BeginCombo("Views", selectedView.name())) {
        for (const auto& [view, _] : ViewFabrics) {
            const auto selected = (index == selectedIndex);
            if (ImGui::Selectable(view.name(), selected)) {
                selectedIndex = index;
                selectedView = view;
            }
            index++;
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Push")) {
        Push(selectedView);
    }
    ImGui::SameLine();
    if (ImGui::Button("Pop"))
        Pop();

    ImGui::End();
}
