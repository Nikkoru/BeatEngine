#pragma once

#include <memory>
#include <functional>
#include <typeindex>
#include <map>
#include <vector>

#include "BeatEngine/Base/Signal.h"

class SignalManager {
public:
	using Callback = std::function<void(const std::shared_ptr<Base::Signal>)>;
private:
	static std::shared_ptr<SignalManager> m_Instance;
	std::map<std::type_index, std::map<std::type_index, std::vector<Callback>>> m_SignalCallbacks;
public:
	SignalManager() = default;
	~SignalManager() = default;
	static std::shared_ptr<SignalManager> GetInstance();
public:
	template<typename TSignal>
		requires (std::is_base_of_v<Base::Signal, TSignal>)
	void RegisterCallback(std::type_index id, Callback callback); 
	void Send(std::shared_ptr<Base::Signal> sig);
};

#include "BeatEngine/Manager/SignalManager.inl"
