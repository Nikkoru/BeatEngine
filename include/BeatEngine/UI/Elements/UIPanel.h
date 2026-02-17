#pragma once

#include "BeatEngine/UI/UIElement.h"

class UIPanel : public UIElement {
private:
    // Color m_ColorRect = Color::White;
    bool m_ShowRect = true;
public:
    UIPanel() = default;
    void OnDraw(/*RenderTarget& target, RenderStates states*/) const /*override*/;
    void OnUpdate(float dt) override {}

    void ShowRect(bool show);
};
