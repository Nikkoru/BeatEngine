#include "BeatEngine/Graphics/FramePacer.hpp"

#include <thread>

#include "BeatEngine/System/Time.h"
#include "BeatEngine/Util/Profiler.h"
#include "BeatEngine/Logger.h"

void FramePacer::PaceFrame(double targetFPS) {
    Profiler::StartProfile({ typeid(FramePacer), "PaceFrame" }, { 0.5f, 0.5f, 0.f, 1.f });
    const auto targetFrameTime = Time{ std::chrono::duration<double>{ 1.0 / targetFPS } };

    auto frameDuration = m_Clock.GetAndReset();
    const auto sleepTime = targetFrameTime - frameDuration;
    
    if (sleepTime > Time{}) {
        std::this_thread::sleep_for(sleepTime.GetDuration());
        Logger::AddDebug(
            typeid(FramePacer),
            "Sleeping for {}",
            std::chrono::duration_cast<std::chrono::milliseconds>(sleepTime.GetDuration())
        );
    }
    else {
        Logger::AddDebug(
            typeid(FramePacer),
            "Not sleeping. frameDuration ({}) was more than targetFrameTime ({})",
            std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration.GetDuration()),
            std::chrono::duration_cast<std::chrono::milliseconds>(targetFrameTime.GetDuration())
        );
    }
    Profiler::EndProfile({ typeid(FramePacer), "PaceFrame" });
}
