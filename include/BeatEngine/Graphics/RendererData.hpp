#pragma once

class Renderer;
class RendererData {
private:
    friend class Renderer;
public:
    RendererData() = default;
    virtual ~RendererData() = default;
};
