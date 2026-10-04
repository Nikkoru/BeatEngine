#include "BeatEngine/Windows/SDL/Cursor.hpp"

#include <SDL3/SDL_mouse.h>

namespace {
SDL_SystemCursor TypeToSDL(CursorType type) {
    switch (type) {
    case Arrow:
        return SDL_SYSTEM_CURSOR_DEFAULT;
    case ArrowWait:
        return SDL_SYSTEM_CURSOR_PROGRESS;
    case Wait:
        return SDL_SYSTEM_CURSOR_WAIT;
    case Text:
        return SDL_SYSTEM_CURSOR_TEXT;
    case Crosshair:
        return SDL_SYSTEM_CURSOR_CROSSHAIR;
    case ResizeNWSE:
        return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    case ResizeNESW:
        return SDL_SYSTEM_CURSOR_NESW_RESIZE;
    case ResizeEW:
        return SDL_SYSTEM_CURSOR_EW_RESIZE;
    case ResizeNS:
        return SDL_SYSTEM_CURSOR_NS_RESIZE;
    case ResizeN:
        return SDL_SYSTEM_CURSOR_N_RESIZE;
    case ResizeNE:
        return SDL_SYSTEM_CURSOR_NE_RESIZE;
    case ResizeE:
        return SDL_SYSTEM_CURSOR_E_RESIZE;
    case ResizeSE:
        return SDL_SYSTEM_CURSOR_SE_RESIZE;
    case ResizeS:
        return SDL_SYSTEM_CURSOR_S_RESIZE;
    case ResizeSW:
        return SDL_SYSTEM_CURSOR_SW_RESIZE;
    case ResizeW:
        return SDL_SYSTEM_CURSOR_W_RESIZE;
    case Move:
        return SDL_SYSTEM_CURSOR_MOVE;
    case NotAllowed:
        return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    case Pointer:
        return SDL_SYSTEM_CURSOR_POINTER;
    case Help:
    case Cell:
    case VerticalText:
    case Alias:
    case Copy:
    case Grab:
    case Grabbing:
    case ZoomIn:
    case ZoomOut:
        return SDL_SYSTEM_CURSOR_DEFAULT;
    }
}
}

SDLCursor::~SDLCursor() {
    SDL_DestroyCursor(_Cursor);
    SDL_DestroySurface(_Surface);
}

void SDLCursor::CreateFromSystem(CursorType type) {
    _Cursor = SDL_CreateSystemCursor(TypeToSDL(type));
}

void SDLCursor::CreateFromPixels(const uint8_t* pixels, Vector2u size, Vector2u hotspot) {
    auto* io = SDL_IOFromConstMem(pixels, size.X * size.Y);
    _Surface = SDL_LoadSurface_IO(io, true);
    _Cursor = SDL_CreateColorCursor(_Surface, hotspot.X, hotspot.Y);
}
