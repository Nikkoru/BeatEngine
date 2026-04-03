#pragma once

#include <deque>
#include <functional>
class UninitQueue {
private:
    std::deque<std::function<void()>> m_UninitCallbacks;
public:
    void AddCallback(std::function<void()>&& callback);
    void Flush();
};
