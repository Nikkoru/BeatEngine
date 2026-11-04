#pragma once

#include "BeatEngine/System/Clock.h"

class FramePacer {
private:
    Clock m_Clock;
public:
    FramePacer() = default;
    ~FramePacer() = default;

    void PaceFrame(double targetFPS = 60.f);
};
