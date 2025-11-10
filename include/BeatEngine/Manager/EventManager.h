#pragma once

#include "BeatEngine/Base/Event.h"

#include <functional>
#include <typeindex>
#include <memory>
#include <unordered_map>

class EventManager {
public:
	using Callback = std::function<void(const std::shared_ptr<Base::Event>&)>;
	using ExitCallback = std::function<void()>;
private:
	static std::shared_ptr<EventManager> m_Instance;
public:
	EventManager() = default;
	~EventManager() = default;
	static std::shared_ptr<EventManager> GetInstance();
public:
	template<typename TEvent>
		requires(std::is_base_of_v<Base::Event, TEvent>)
	void SubscribeView(std::type_index id, Callback callback) {
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
	void Subscribe(Callback callback) {
		auto eventID = std::type_index(typeid(TEvent));

		if (!m_Subscribers.contains(eventID))
			m_Subscribers[eventID];
		m_Subscribers.at(eventID).emplace_back(std::move(callback));
	}
	void Send(const std::shared_ptr<Base::Event> event);

	void SetExitCallback(ExitCallback callback);
protected:
	friend class ViewManager;
	void UpdateMainView(const std::type_index id);
private:
	std::unordered_map<std::type_index, std::unordered_map<std::type_index, std::vector<Callback>>> m_ViewSubscribers;
	std::unordered_map<std::type_index, std::vector<Callback>> m_Subscribers;
	std::type_index m_MainView = typeid(nullptr);
private:
	ExitCallback m_ExitCallback;
};