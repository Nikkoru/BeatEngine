#include "BeatEngine/Util/Profiler.h"
#include "BeatEngine/ImGui/MultiPlot.h"
#include "BeatEngine/System/Time.h"
#include "BeatEngine/Util/CountedArray.h"
#include "imgui.h"
#include <cstddef>
#include <memory>

std::shared_ptr<Profiler> Profiler::m_Instance = nullptr;

bool Profiler::HasHighlight() {
    for (const auto& [fdata, pdata] : GetInstance()->m_Deltas)
        if (pdata.Highlight)
            return true;

    return false;
}

std::shared_ptr<Profiler> Profiler::GetInstance() {
    if (!m_Instance)
        m_Instance = std::make_shared<Profiler>();
        
    return m_Instance;
}

void PlotMultiHistogramsHighlights(const char *label, int num_hists, const char **names,
    const ImColor* colors, float(*getter)(const void *data, int idx, int tableIndex),
    float(*getterToolTip)(const void *data, int idx, int tableIndex),
    const void *datas, int values_count, float scale_min, float scale_max, ImVec2 graph_size) {
    
    auto map = (std::map<FunctionData, ProfilerData>*)datas;
    
    std::vector<ImColor> realColors;
    
    for (int i = 0; i < num_hists; i++) {
        realColors.emplace_back(colors[i]);
    }
     
    auto index = 0;
    if (Profiler::HasHighlight())
        for (const auto& [fdata, pdata] : *map) {
            if (!pdata.Highlight)
                realColors[index].Value.w = 0.2f;
            index++;
        }

    PlotMultiHistograms(label, num_hists, names, realColors.data(), getter, getterToolTip, datas, values_count, scale_min, scale_max, graph_size);
}

void Profiler::StartProfile(FunctionData data, ImColor color) {
    if (!GetInstance()->m_Deltas.contains(data)) {
        GetInstance()->m_Deltas[data];
        GetInstance()->m_Deltas.at(data).Deltas.SetAll(-1);
        GetInstance()->m_Deltas.at(data).Color = color;
    }

    GetInstance()->m_Deltas.at(data).DeltaClock.Start();
}

void Profiler::EndProfile(FunctionData data) {
    auto delta = GetInstance()->m_Deltas.at(data).DeltaClock.GetAndStop().AsMilliseconds();

    if (delta > GetInstance()->m_Deltas.at(data).MaxDelta)
        GetInstance()->m_Deltas.at(data).MaxDelta = delta;

    if (GetInstance()->m_Deltas.at(data).Deltas.Full()) {
        auto array = GetInstance()->m_Deltas.at(data).Deltas.Data();
        
        std::move(array + 1, array + 30, array);
        array[30 - 1] = delta;

        GetInstance()->m_Deltas.at(data);
    }
    else {
        GetInstance()->m_Deltas.at(data).Deltas.Add(delta);
    }
}

void Profiler::DeleteProfileEntry(FunctionData data) {
    if (GetInstance()->m_Deltas.contains(data))
        GetInstance()->m_Deltas.erase(data);
}

CountedArray<float, 30> Profiler::GetDeltas(FunctionData data) {
    if (GetInstance()->m_Deltas.contains(data))
        return GetInstance()->m_Deltas.at(data).Deltas;
    else
        return {};
}

float Profiler::GetMaxDelta(FunctionData data) {
    if (GetInstance()->m_Deltas.contains(data))
        return GetInstance()->m_Deltas.at(data).MaxDelta;
    else
        return {};
}

float plotGetter(const void* data, int index, int tableIndex) {
    auto mapData = (std::map<FunctionData, ProfilerData>*)data;
    auto it = mapData->begin();
    for (int i = 0; i < tableIndex; i++)
        it++;
    if (it->second.Show)
        return (it)->second.Deltas[index];
    else
        return -2;
}

void Profiler::DrawHistogram(ImVec2 size) {
    std::vector<const char*> names;
    std::vector<ImColor> colors;
    float maxDelta{};

    for (const auto& [fdata, pdata] : GetInstance()->m_Deltas) {
        if (pdata.MaxDelta > maxDelta && pdata.Show)
            maxDelta = pdata.MaxDelta;
        names.emplace_back(fdata.second.c_str());
        colors.emplace_back(pdata.Color);
    }

    PlotMultiHistogramsHighlights(
        "##ProfilerHistogram",
        GetInstance()->m_Deltas.size(),
        names.data(),
        colors.data(),
        plotGetter,
        plotGetter,
        &GetInstance()->m_Deltas,
        30,
        0,
        maxDelta,
        { size.x, ( size.y - (ImGui::CalcTextSize(names[0]).y * 2) * GetInstance()->m_Deltas.size()) - 5 }
    );
    ImGui::Text("Max delta: %f", maxDelta);
    ImGui::SameLine();
    if (ImGui::SmallButton("Show All"))
        ShowAll();

    bool hovered = false;
    for (size_t index = 0; index < GetInstance()->m_Deltas.size(); index++) {
        float average{};
        auto it = GetInstance()->m_Deltas.begin();
        for (size_t i = 0; i < index; i++)
            it++;

        for (const auto& delta : it->second.Deltas)
            average += delta; 
        average /= it->second.Deltas.UsedSize();

        ImGui::Checkbox(std::format("##Show_{}", it->first.second).c_str(), &it->second.Show);
        ImGui::SameLine();

        ImGui::BeginDisabled(!it->second.Show);
        ImGui::ColorButton(std::format("##ProfilerColorButton{}", index).c_str(), colors[index], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
        if (ImGui::IsItemHovered() && it->second.Show) {
            Profiler::Highlight(it->first);
            hovered = true;
        }
        else if (!hovered)
            Profiler::StopHighlight();

        ImGui::SameLine();

        ImGui::TextColored({.3f, .3f, .3f, 1.0f}, "%s |", it->first.first.name());
        if (ImGui::IsItemHovered() && it->second.Show) {
            Profiler::Highlight(it->first.first);
            hovered = true;
        }
        else if (!hovered)
            Profiler::StopHighlight();

        ImGui::SameLine();

        ImGui::Text("%s: Avg: %fms", names[index], average);
        if (ImGui::IsItemHovered() && it->second.Show) {
            Profiler::Highlight(it->first);
            hovered = true;
        }
        else if (!hovered)
            Profiler::StopHighlight();

        ImGui::SameLine();

        ImGui::Text("Max: %fms", it->second.MaxDelta);
        if (ImGui::IsItemHovered() && it->second.Show) {
            Profiler::Highlight(it->first);
            hovered = true;
        }
        else if (!hovered)
            Profiler::StopHighlight();
        ImGui::EndDisabled();
    }
}

void Profiler::Filter(std::type_index object){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas) {
        if (fdata.first == object)
            pdata.Show = false;
    }
}

void Profiler::Filter(std::string funcName){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas) {
        if (fdata.second == funcName)
            pdata.Show = false;
    }
}

void Profiler::Filter(FunctionData funcData){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas) {
        if (fdata == funcData)
            pdata.Show = false;
    }
}


void Profiler::Highlight(std::type_index object){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas) {
        if (fdata.first == object)
            pdata.Highlight = true;
    }
}

void Profiler::Highlight(std::string funcName){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas) {
        if (fdata.second == funcName)
            pdata.Highlight = true;
    }
}

void Profiler::Highlight(FunctionData funcData){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas) {
        if (fdata == funcData)
            pdata.Highlight = true;
    }
}


void Profiler::ShowAll(){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas)
        pdata.Show = true;
}

void Profiler::StopHighlight(){
    for (auto& [fdata, pdata] : GetInstance()->m_Deltas)
        pdata.Highlight = false;
}
