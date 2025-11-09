#pragma once

#include "Base/Event.h"

#include <functional>
#include <typeindex>
#include <memory>
#include <unordered_map>

class EventManager {
public:
	using Callback = std::function<void(const std::shared_ptr<Base::Event>&)>;
	using ExitCallback = std::function<void()>;
private:
	std::shared_ptr<EventManager> m_Instance = nullptr;
public:
	EventManager() = default;
	~EventManager() = default;
	std::shared_ptr<EventManager> GetInstance();
public:
	template<typename TEvent>
		requires(std::is_base_of_v<Base::Event, TEvent>)
	void SubscribeView(std::type_index id, Callback callback);
	template<typename TEvent>
		requires(std::is_base_of_v<Base::Event, TEvent>)
	void Subscribe(Callback callback);
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