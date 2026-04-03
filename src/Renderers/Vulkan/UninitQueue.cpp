#include "BeatEngine/Renderers/Vulkan/UninitQueue.h"
#include <functional>

void UninitQueue::AddCallback(std::function<void()>&& callback) {
    m_UninitCallbacks.emplace_back(callback);
}

void UninitQueue::Flush() {
    for (auto it = m_UninitCallbacks.rbegin(); it != m_UninitCallbacks.rend(); it++) {
        (*it)();
    }

    m_UninitCallbacks.clear();
}
