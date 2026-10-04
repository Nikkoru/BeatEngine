#include "BeatEngine/Manager/SignalManager.h"

#include "BeatEngine/Logger.h"
#include "imgui.h"

std::shared_ptr<SignalManager> SignalManager::m_Instance = nullptr;

std::shared_ptr<SignalManager> SignalManager::GetInstance() {
	if (m_Instance == nullptr)
		m_Instance = std::make_shared<SignalManager>();
	return m_Instance;
}

void SignalManager::Send(std::shared_ptr<Base::Signal> sig) {
	if (!m_SignalCallbacks.empty())
		if (m_SignalCallbacks.contains(sig->ID))
			for (const auto& [index, vector] : m_SignalCallbacks.at(sig->ID))
				for (const auto& callback : vector)
					callback(sig);
		else
			Logger::AddWarning(typeid(SignalManager), "No callbacks registered for signal");
	else
		Logger::AddWarning(typeid(SignalManager), "No signal callbacks registered");
}

void SignalManager::ShowImGuiDebugWindow() {
    ImGui::Begin("SignalManager Debug Window"); 
    ImGui::Text("Callbacks: %zu", m_SignalCallbacks.size());
    for (const auto& [signalID, callbackMap] : m_SignalCallbacks) {
        if (ImGui::TreeNode(signalID.name())) {
            for (const auto& [callerID, callbacks] : callbackMap) {
                if (ImGui::TreeNode(callerID.name())) {
                    ImGui::Text("Callbacks: %zu", callbacks.size());
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}
