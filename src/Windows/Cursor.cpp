#include "BeatEngine/Windows/Cursor.hpp"
#include "BeatEngine/Graphics/BaseWindow.h"

Optional<Cursor> Cursor::CreateFromSystem(std::shared_ptr<BaseWindow> window, CursorType type) {
    auto cursor = window->CreateCursor();
    cursor.m_Impl->CreateFromSystem(type);
    return cursor;
}
