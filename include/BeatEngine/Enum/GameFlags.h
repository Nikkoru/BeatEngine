#pragma once

using GameFlags = int;

enum GameFlags_ {
    GameFlags_None                      = 0,
    GameFlags_ImGui                     = 1 << 0,
    GameFlags_ImGuiDocking              = 1 << 0,
    GameFlags_Running                   = 1 << 2,
    GameFlags_Preload                   = 1 << 3,
    GameFlags_Fullscreen                = 1 << 4,
    GameFlags_CursorChanged             = 1 << 5,
    GameFlags_DisableKeyPressEvents     = 1 << 6,
    GameFlags_DrawDebugInfo             = 1 << 7,
};
