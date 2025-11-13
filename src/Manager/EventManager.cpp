#include "BeatEngine/Manager/EventManager.h"

#include "BeatEngine/Logger.h"


std::shared_ptr<EventManager> EventManager::m_Instance = nullptr;

std::shared_ptr<EventManager> EventManager::GetInstance() {
	if (m_Instance == nullptr)
		m_Instance = std::make_shared<EventManager>();
	return m_Instance;
}

void EventManager::Send(const std::shared_ptr<Base::Event> event) {
	auto eventID = event->ID;
	if (!m_ViewSubscribers.empty())
		if (m_ViewSubscribers.contains(m_MainView))
			if (m_ViewSubscribers.at(m_MainView).contains(eventID))
				for (Callback& callback : m_ViewSubscribers.at(m_MainView).at(eventID))
					callback(event);
			else
				Logger::GetInstance()->AddWarning("No subscribers for event in main view", typeid(EventManager));
		else
			Logger::GetInstance()->AddWarning("No subscribers for main view", typeid(EventManager));
	else
		Logger::GetInstance()->AddWarning("No view subscribers registered", typeid(EventManager));

	if (!m_Subscribers.empty())
		if (m_Subscribers.contains(eventID))
			for (Callback& callback : m_Subscribers.at(eventID))
				callback(event);
		else
			Logger::GetInstance()->AddWarning("No global subscribers for event", typeid(EventManager));
	else
		Logger::GetInstance()->AddWarning("No global subscribers registered", typeid(EventManager));

	if (eventID == typeid(GameExitEvent) && m_ExitCallback)
		m_ExitCallback();
}

void EventManager::SetExitCallback(ExitCallback callback) {
	m_ExitCallback = std::move(callback);
}

void EventManager::UpdateMainView(std::type_index id) {
	m_MainView = id;
}