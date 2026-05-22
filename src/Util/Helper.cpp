#include "BeatEngine/Util/Helper.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "imgui.h"
#include <cstring>

bool Helper::ParseBoolFromString(const char *str) {
    return strlen(str) == 4;
}

bool Helper::RectHovered(Vector2f posMin, Vector2f posMax, Vector2i mousePos, bool showCondition) {
    if (mousePos == Vector2i{ -1, -1 })
        mousePos = { static_cast<int>(ImGui::GetMousePos().x), static_cast<int>(ImGui::GetMousePos().y) };

    if (showCondition) {
        ImGui::BeginTooltip();
        if (mousePos.X > posMin.X)
            ImGui::PushStyleColor(ImGuiCol_Text, { .0f, 1.0f, .0f, 1.0f });
        else
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, .0f, .0f, 1.0f });
        ImGui::Text("(%d) > (%f)", mousePos.X, posMin.X);
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (mousePos.Y > posMin.Y)
            ImGui::PushStyleColor(ImGuiCol_Text, { .0f, 1.0f, .0f, 1.0f });
        else
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, .0f, .0f, 1.0f });
        ImGui::Text("(%d) < ( %f)", mousePos.Y, posMin.Y);
        ImGui::PopStyleColor();


        if (mousePos.X < posMax.X)
            ImGui::PushStyleColor(ImGuiCol_Text, { .0f, 1.0f, .0f, 1.0f });
        else
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, .0f, .0f, 1.0f });
        ImGui::Text("(%d) < (%f)", mousePos.X,  posMax.X);
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (mousePos.Y < posMax.Y)
            ImGui::PushStyleColor(ImGuiCol_Text, { .0f, 1.0f, .0f, 1.0f });
        else
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, .0f, .0f, 1.0f });
        ImGui::Text("(%d) > (%f)", mousePos.Y, posMax.Y);
        ImGui::PopStyleColor();


        ImGui::EndTooltip();
    }

    return 
        mousePos.X > posMin.X && mousePos.X < posMax.X &&
        mousePos.Y > posMin.Y && mousePos.Y < posMax.Y;
}
