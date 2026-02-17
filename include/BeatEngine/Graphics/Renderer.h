#pragma once
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <memory>
class Renderer {
protected:
    std::shared_ptr<BaseWindow> m_Window;
public:
    Renderer() = default;
    virtual ~Renderer() = default;
public:
    virtual void Init(std::string windowTitle, Vector2u windowSize) = 0;
    virtual void Uninit() = 0;
    virtual void Render() = 0;
    virtual void Display() = 0;
    virtual void Clear() = 0;
    virtual void Update() = 0;

    inline std::shared_ptr<BaseWindow> GetWindow() { return m_Window; }

    virtual std::optional<Base::Event> PollEvent() const = 0;
};
