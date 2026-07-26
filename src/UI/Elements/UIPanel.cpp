#include "BeatEngine/UI/Elements/UIPanel.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"

void UIPanel::OnDraw(GraphicsManager& mgr, RenderState state) {
   // if (m_ShowRect)
   //     target.draw(m_LayoutRect);
}

void UIPanel::ShowRect(bool show) {
    this->m_ShowRect = show;
}
