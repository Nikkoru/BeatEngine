#include "BeatEngine/Manager/SignalManager.h"

#include "BeatEngine/Logger.h"

std::shared_ptr<SignalManager> SignalManager::m_Instance = nullptr;

std::shared_ptr<SignalManager> SignalManager::GetInstance() {
	if (m_Instance == nullptr)
		m_Instance = std::make_shared<SignalManager>();
	return m_Instance;
}

void SignalManager::Send(std::shared_ptr<Base::Signal> sig) {
	if (!m_SignalCallbacks.empty())
		if (m_SignalCallbacks.contains(sig->ID))
			for (auto& [index, vector] : m_SignalCallbacks.at(sig->ID))
				for (auto& callback : vector)
					callback(sig);
		else
			Logger::GetInstance()->AddWarning("No callbacks registered for signal", typeid(SignalManager));
	else
		Logger::GetInstance()->AddWarning("No signal callbacks registered", typeid(SignalManager));
}
