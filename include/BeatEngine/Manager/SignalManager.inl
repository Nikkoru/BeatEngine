#include "BeatEngine/Manager/SignalManager.h"

template<typename TSignal>
    requires (std::is_base_of_v<Base::Signal, TSignal>)
void SignalManager::RegisterCallback(std::type_index id, Callback callback) {
    auto signalID = std::type_index(typeid(TSignal));

    if (m_SignalCallbacks.contains(signalID))
        if (m_SignalCallbacks.at(signalID).contains(id))
            m_SignalCallbacks.at(signalID).at(id).emplace_back(callback);
        else
            m_SignalCallbacks.at(signalID).try_emplace(id, std::vector<Callback>{ callback });
    else
        m_SignalCallbacks.try_emplace(signalID, std::map<std::type_index, std::vector<Callback>>{ { id, std::vector<Callback>{ callback } } });
}
