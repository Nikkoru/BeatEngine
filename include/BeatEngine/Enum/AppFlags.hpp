#pragma once

using AppFlags = int;

enum AppFlags_ {
    AppFlags_None                      = 0,
    AppFlags_ImGui                     = 1 << 0,
    AppFlags_ImGuiDocking              = 1 << 0,
    AppFlags_Running                   = 1 << 2,
    AppFlags_Preload                   = 1 << 3,
    AppFlags_Fullscreen                = 1 << 4,
    AppFlags_CursorChanged             = 1 << 5,
    AppFlags_DisableKeyPressEvents     = 1 << 6,
    AppFlags_DrawDebugInfo             = 1 << 7,
    AppFlags_DebugDock                 = 1 << 8,
};
