#include "BeatEngine/Manager/EventManager.h"

template<typename TEvent>
    requires(std::is_base_of_v<Base::Event, TEvent>)
void EventManager::SubscribeView(std::type_index id, Callback callback) {
    if (id == typeid(nullptr)) {
        Logger::AddWarning(typeid(EventManager), "Attempted to subscribe null view to event");
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

