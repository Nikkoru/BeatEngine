#include "Manager/EventManager.h"

#include "Logger.h"

template<typename TEvent>
	requires(std::is_base_of_v<Base::Event, TEvent>)
void EventManager::SubscribeView(std::type_index id, Callback callback) {
	if (id == typeid(nullptr)) {
		Logger::GetInstance()->AddWarning("Attempted to subscribe null view to event", typeid(EventManager));
		return;
	}

	auto eventID = std::type_index(typeid(TEvent));

	if (!m_ViewSubscribers.contains(id))
		m_ViewSubscribers[id];

	if (m_ViewSubscribers.at(id).find(eventID) == m_ViewSubscribers.at(id).end()) {
		m_ViewSubscribers.at(id)[eventID] = {};
	}

	m_ViewSubscribers.at(id).at(eventID).emplace_back(std::move(callback));
}

template<typename TEvent>
	requires(std::is_base_of_v<Base::Event, TEvent>)
void EventManager::Subscribe(Callback callback) {
	auto eventID = std::type_index(typeid(TEvent));

	if (!m_Subscribers.contains(eventID))
		m_Subscribers[eventID];
	m_Subscribers.at(eventID).emplace_back(std::move(callback));
}

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
}

void EventManager::SetExitCallback(ExitCallback callback) {
	m_ExitCallback = std::move(callback);
}

void EventManager::UpdateMainView(std::type_index id) {
	m_MainView = id;
}