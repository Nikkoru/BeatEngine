#pragma once

#include "BeatEngine/Graphics/RenderState.hpp"

class Renderer;
class RendererData;
class GraphicalElement {
private:
    friend class GraphicsManager;
private:
    // this is meant to be used by the renderer, in the case of vulkan it needs to know what
    // element is this, so it can use the correct buffers binded to the id
    uint32_t m_ElementID;
    // also we need to know if it is already initialized so we can know if the requested id
    // needs to be freed or not, this overrides whatever it was in that id without questions
    bool m_Initialized{ false };
protected:
public:
    GraphicalElement() = default;
    virtual ~GraphicalElement() = default;

    virtual void Draw(Renderer* const mgr, RenderState state = RenderState::Default) = 0;
};
