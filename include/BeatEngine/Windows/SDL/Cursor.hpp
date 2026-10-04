#pragma once

#include "BeatEngine/Windows/Cursor.hpp"
#include <SDL3/SDL_mouse.h>

class SDLCursor : public CursorImpl {
public:
    SDL_Cursor* _Cursor = nullptr;
    SDL_Surface* _Surface = nullptr;
public:
    ~SDLCursor() override;

    void CreateFromSystem(CursorType type) override;
    void CreateFromPixels(const uint8_t* pixels, Vector2u size, Vector2u hotspot) override;
};
