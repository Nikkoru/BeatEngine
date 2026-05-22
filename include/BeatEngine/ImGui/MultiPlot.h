#pragma once

#include <cfloat>
#include <imgui_internal.h>

ImU32 InvertColorU32(ImU32 in);

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
    float scaleMin = FLT_MAX,
    float scaleMax = FLT_MAX,
    ImVec2 rectSize = { .0f, .0f });

void PlotMultiLines(
    const char *label,
    int num_datas,
    const char **names,
    const ImColor *colors,
    float(*getter)(const void *data, int idx, int tableIndex),
    float(*getterToolTip)(const void *data, int idx, int tableIndex),
    const void *datas,
    int values_count,
    float scale_min,
    float scale_max,
    ImVec2 graph_size);

void PlotMultiHistograms(
    const char *label,
    int num_hists,
    const char **names,
    const ImColor *colors,
    float(*getter)(const void *data, int idx, int tableIndex),
    float(*getterToolTip)(const void *data, int idx, int tableIndex),
    const void *datas,
    int values_count,
    float scale_min,
    float scale_max,
    ImVec2 graph_size);

// https://github.com/meemknight/ourCraft/blob/master/thirdparty/imgui-docking/imgui/multiPlot.h
// https://github.com/meemknight/ourCraft/blob/master/thirdparty/imgui-docking/imgui/multiPlot.cpp
