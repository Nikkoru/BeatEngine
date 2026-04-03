#pragma once

using GameFlags = int;

enum GameFlags_ {
    GameFlags_None                      = 0,
    GameFlags_ImGui                     = 1 << 0,
    GameFlags_Running                   = 1 << 1,
    GameFlags_Preload                   = 1 << 2,
    GameFlags_Fullscreen                = 1 << 3,
    GameFlags_CursorChanged             = 1 << 4,
    GameFlags_DisableKeyPressEvents     = 1 << 5,
    GameFlags_DrawDebugInfo             = 1 << 6,
};
