#include "BeatEngine/ImGui/MultiPlot.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Helper.h"
#include "imgui.h"

ImU32 InvertColorU32(ImU32 in)
{
    ImVec4 in4 = ImGui::ColorConvertU32ToFloat4(in);
    in4.x = 1.f - in4.x;
    in4.y = 1.f - in4.y;
    in4.z = 1.f - in4.z;
    return ImGui::GetColorU32(in4);
}

void PlotMultiEx(
    ImGuiPlotType plotType,
    const char* label,
    int datasCount,
    const char** dataNames,
    const ImColor* colors,
    float(*valueGetter)(const void *data, int idx, int tableIndex),
    float(*valueGetterToolTip)(const void *data, int idx, int tableIndex),
    const void* datas,
    int dataSize,
    float scaleMin,
    float scaleMax,
    ImVec2 rectSize
) {
    auto prevObjPos = ImGui::GetItemRectMin();
    auto prevObjSize = ImGui::GetItemRectSize();
    const int valOffset = 0;

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;

    const ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);

    if (rectSize.x == 0.0f)
        rectSize.x = ImGui::CalcItemWidth();
    if (rectSize.y == 0.0f)
        rectSize.y = labelSize.y + (style.FramePadding.y * 2);

    const ImRect frameRect(window->DC.CursorPos, { window->DC.CursorPos.x + rectSize.x, window->DC.CursorPos.y + rectSize.y });
    const ImRect innerRect({ frameRect.Min.x + style.FramePadding.x, frameRect.Min.y + style.FramePadding.x }, { frameRect.Max.x - style.FramePadding.x, frameRect.Max.y - style.FramePadding.y });
    const ImRect totalRect(frameRect.Min, { frameRect.Max.x - labelSize.x > 0.0f ? style.ItemInnerSpacing.x + labelSize.x : 0.0f, 0 });
    ImGui::ItemSize(totalRect, style.FramePadding.y);
    if (!ImGui::ItemAdd(totalRect, 0))
        return;

    // Determine scale from values if not specified
    if (scaleMin == FLT_MAX || scaleMax == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int data_idx = 0; data_idx < datasCount; ++data_idx)
        {
            for (int i = 0; i < dataSize; i++)
            {
                const float v = valueGetter(datas, i, data_idx);
                v_min = ImMin(v_min, v);
                v_max = ImMax(v_max, v);
            }
        }
        if (scaleMin == FLT_MAX)
            scaleMin = v_min;
        if (scaleMax == FLT_MAX)
            scaleMax = v_max;
    }

    ImGui::RenderFrame(frameRect.Min, frameRect.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    int res_w = ImMin((int)rectSize.x, dataSize) + ((plotType == ImGuiPlotType_Lines) ? -1 : 0);
    int itemCount = dataSize + ((plotType == ImGuiPlotType_Lines) ? -1 : 0);

    // Tooltip on hover
    int hovered = -1;
    if (ImGui::IsItemHovered() || Helper::RectHovered({ frameRect.Min.x, frameRect.Min.y }, { frameRect.Max.x, frameRect.Max.y }))
    {
        const float t = ImClamp((g.IO.MousePos.x - innerRect.Min.x) / (innerRect.Max.x - innerRect.Min.x), 0.0f, 0.9999f);
        const int index = (int)(t * itemCount);
        IM_ASSERT(index >= 0 && index < dataSize);

        // std::string toolTip;
        ImGui::BeginTooltip();

        //enlarge
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // Increase padding
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));  // Increase frame padding
        // ImGui::SetWindowFontScale(1.5f); // Scale the font size for the tooltip

        const int idx0 = (index + valOffset) % dataSize;
        if (plotType == ImGuiPlotType_Lines)
        {
            const int idx1 = (index + 1 + valOffset) % dataSize;
            ImGui::Text("%8d %8d | Name", index, index + 1);
            for (int dataIdx = 0; dataIdx < datasCount; ++dataIdx)
            {
                const float v0 = valueGetterToolTip(datas, idx0, dataIdx);
                const float v1 = valueGetterToolTip(datas, idx1, dataIdx);
                if (v1 != -2 || v0 != -2)
                    ImGui::TextColored(colors[dataIdx], "%f %f | %s", v0, v1, dataNames[dataIdx]);
            }
        }
        else if (plotType == ImGuiPlotType_Histogram)
        {
            for (int dataIdx = 0; dataIdx < datasCount; ++dataIdx)
            {
                const float v0 = valueGetterToolTip(datas, idx0, dataIdx);
                if (v0 != -2)
                    ImGui::TextColored(colors[dataIdx], "%d: %f | %s", index, v0, dataNames[dataIdx]);
            }
        }

        ImGui::SetWindowFontScale(1.0f); // Reset font scale to default
        ImGui::PopStyleVar(2);

        ImGui::EndTooltip();
        hovered = index;
    }

    for (int data_idx = 0; data_idx < datasCount; ++data_idx)
    {
        const float t_step = 1.0f / (float)res_w;

        float v0 = valueGetter(datas, (0 + valOffset) % dataSize, data_idx);
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2(t0, 1.0f - ImSaturate((v0 - scaleMin) / (scaleMax - scaleMin)));    // Point in the normalized space of our target rectangle

        const ImU32 colBase = colors[data_idx];
        const ImU32 colHovered = InvertColorU32(colors[data_idx]);

        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * itemCount + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < dataSize);
            const float v1 = valueGetter(datas, (v1_idx + valOffset + 1) % dataSize, data_idx);
            const ImVec2 tp1 = ImVec2(t1, 1.0f - ImSaturate((v1 - scaleMin) / (scaleMax - scaleMin)));

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(innerRect.Min, innerRect.Max, tp0);
            ImVec2 pos1 = ImLerp(innerRect.Min, innerRect.Max, (plotType == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, 1.0f));
            if (plotType == ImGuiPlotType_Lines)
            {
                window->DrawList->AddLine(pos0, pos1, hovered == v1_idx ? colHovered : colBase);
            }
            else if (plotType == ImGuiPlotType_Histogram)
            {
                if (pos1.x >= pos0.x + 2.0f)
                    pos1.x -= 1.0f;
                window->DrawList->AddRectFilled(pos0, pos1, hovered == v1_idx ? colHovered : colBase);
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    ImGui::RenderText(ImVec2(frameRect.Max.x + style.ItemInnerSpacing.x, innerRect.Min.y), label);

    ImGui::SetCursorScreenPos({ window->Pos.x + window->WindowPadding.x, prevObjPos.y + prevObjSize.y + rectSize.y + style.FramePadding.y * 3 });
}

void PlotMultiLines(const char *label, int num_datas, const char **names,
    const ImColor *colors, float(*getter)(const void *data, int idx, int tableIndex),
    float(*getterToolTip)(const void *data, int idx, int tableIndex),
    const void *datas, int values_count, float scale_min, float scale_max, ImVec2 graph_size)
{
    PlotMultiEx(ImGuiPlotType_Lines, label, num_datas, names, colors, getter,
        getterToolTip, datas, values_count, scale_min, scale_max, graph_size);
}

void PlotMultiHistograms(const char *label, int num_hists, const char **names,
    const ImColor *colors, float(*getter)(const void *data, int idx, int tableIndex),
    float(*getterToolTip)(const void *data, int idx, int tableIndex),
    const void *datas, int values_count, float scale_min, float scale_max, ImVec2 graph_size)
{
    PlotMultiEx(ImGuiPlotType_Histogram, label, num_hists, names, colors, getter,
        getterToolTip, datas, values_count, scale_min, scale_max, graph_size);
}
