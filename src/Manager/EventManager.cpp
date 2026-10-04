#include "BeatEngine/Manager/EventManager.h"

#include "BeatEngine/Logger.h"
#include "imgui.h"


std::shared_ptr<EventManager> EventManager::m_Instance = nullptr;

std::shared_ptr<EventManager> EventManager::GetInstance() {
	if (m_Instance == nullptr)
		m_Instance = std::make_shared<EventManager>();
	return m_Instance;
}

void EventManager::Send(const std::shared_ptr<Base::Event> event) {
	auto eventID = event->ID;
	if (!m_ViewSubscribers.empty())
		if (m_ViewSubscribers.contains(m_MainView)) {
			if (m_ViewSubscribers.at(m_MainView).contains(eventID))
				for (Callback& callback : m_ViewSubscribers.at(m_MainView).at(eventID))
					callback(event);
        }
		else {
			Logger::AddWarning(typeid(EventManager), "No subscribers for main view");
            return;
        }
	else {
		Logger::AddWarning(typeid(EventManager), "No view subscribers registered");
        return;
    }

	if (!m_Subscribers.empty())
		if (m_Subscribers.contains(eventID))
			for (Callback& callback : m_Subscribers.at(eventID))
				callback(event);
}

void EventManager::SetExitCallback(ExitCallback callback) {
	m_ExitCallback = std::move(callback);
}

void EventManager::ShowImGuiDebugWindow() {
    ImGui::Begin("EventManager Debug Window"); 
    ImGui::Text("View Subscribers: %zu", m_ViewSubscribers.size());
    for (const auto& [eventID, subscribersMap] : m_ViewSubscribers) {
        if (ImGui::TreeNode(eventID.name())) {
            for (const auto& [subscriberID, callbacks] : subscribersMap) {
                if (ImGui::TreeNode(subscriberID.name())) {
                    ImGui::Text("Callbacks: %zu", callbacks.size());
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::Text("Global Subscribers: %zu", m_Subscribers.size());
    for (const auto& [eventID, callbacks] : m_Subscribers) {
        if (ImGui::TreeNode(eventID.name())) {
            ImGui::Text("Callbacks: %zu", callbacks.size());
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void EventManager::UpdateMainView(std::type_index id) {
	m_MainView = id;
}
