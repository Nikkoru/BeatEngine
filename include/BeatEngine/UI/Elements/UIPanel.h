#pragma once

#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/UI/UIElement.h"

class UIPanel : public UIElement {
private:
    // Color m_ColorRect = Color::White;
    bool m_ShowRect = true;
public:
    UIPanel() = default;
    void OnDraw(GraphicsManager& mgr) override;
    void OnUpdate(float dt) override { (void)dt; }

    void ShowRect(bool show);
};
