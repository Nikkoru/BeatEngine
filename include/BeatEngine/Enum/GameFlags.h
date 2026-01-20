#pragma once

using GameFlags = int;

enum GameFlags_ {
    GameFlags_None                      = 0,
    GameFlags_ImGui                     = 1 << 0,
    GameFlags_Preload                   = 1 << 1,
    GameFlags_Fullscreen                = 1 << 2,
    GameFlags_CursorChanged             = 1 << 3,
    GameFlags_DisableKeyPressEvents     = 1 << 4
};
