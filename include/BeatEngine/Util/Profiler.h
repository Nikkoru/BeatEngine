#pragma once

#include "BeatEngine/System/Clock.h"
#include "BeatEngine/Util/CountedArray.h"
#include <imgui.h>
#include <map>
#include <typeindex>

using FunctionData = std::pair<std::type_index, std::string>;

struct ProfilerData {
    bool Show{ true };
    bool Highlight{ false };
    Clock DeltaClock{};
    CountedArray<float, 30> Deltas;
    float MaxDelta{};
    ImColor Color;
};

class Profiler {
private:
    std::map<FunctionData, ProfilerData> m_Deltas;
    bool m_Profile = true;
    static std::shared_ptr<Profiler> m_Instance;
public:
    static std::shared_ptr<Profiler> GetInstance();
    static bool HasHighlight();

    static void StartProfile(FunctionData data, ImColor color);
    static void EndProfile(FunctionData data);
    static void DeleteProfileEntry(FunctionData data);

    static CountedArray<float, 30> GetDeltas(FunctionData data);
    static float GetMaxDelta(FunctionData data);

    static void Filter(std::type_index object);
    static void Filter(std::string funcName);
    static void Filter(FunctionData funcData);

    static void Highlight(std::type_index object);
    static void Highlight(std::string funcName);
    static void Highlight(FunctionData funcData);

    static void Show(std::type_index object);
    static void Show(std::string funcName);
    static void Show(FunctionData funcData);

    static void Hide(std::type_index object);
    static void Hide(std::string funcName);
    static void Hide(FunctionData funcData);

    static void ShowAll();
    static void HideAll();
    static void StopHighlight();

    static void ResetMaxDelta(std::type_index object);
    static void ResetMaxDelta(std::string funcName);
    static void ResetMaxDelta(FunctionData funcData);
    static void ResetAllMaxDelta();

    static void Pause();
    static void Resume();

    static void DrawHistogram(ImVec2 size);
};
