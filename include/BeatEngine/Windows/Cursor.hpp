#pragma once

#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Util/Optional.hpp"

class BaseWindow;

enum CursorType {
    Arrow = 0,
    ArrowWait,
    Wait,
    Text,
    Crosshair,
    ResizeNWSE,
    ResizeNESW,
    ResizeEW,
    ResizeNS,
    ResizeN,
    ResizeNE,
    ResizeE,
    ResizeSE,
    ResizeS,
    ResizeSW,
    ResizeW,
    Move,
    NotAllowed,
    Pointer,
    Help,
    Cell,
    VerticalText,
    Alias,
    Copy,
    Grab,
    Grabbing,
    ZoomIn,
    ZoomOut
};

class CursorImpl {
public:
    virtual ~CursorImpl() = default;

    virtual void CreateFromSystem(CursorType type) = 0;
    virtual void CreateFromPixels(const uint8_t* pixels, Vector2u size, Vector2u hotspot) = 0;
};

class Cursor {
private:
    friend class BaseWindow;
    std::shared_ptr<CursorImpl> m_Impl{};
public:
    static Optional<Cursor> CreateFromSystem(std::shared_ptr<BaseWindow> window, CursorType type);
    static Optional<Cursor> CreateFromPixels(std::shared_ptr<BaseWindow> window, const uint8_t* pixels, Vector2u size, Vector2u hotspot);
private:
    std::shared_ptr<CursorImpl> GetImpl() const { return m_Impl; }
};
