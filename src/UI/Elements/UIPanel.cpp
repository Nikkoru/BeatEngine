#include "BeatEngine/UI/Elements/UIPanel.h"

void UIPanel::OnDraw(/*sf::RenderTarget& target, sf::RenderStates states*/) const {
   // if (m_ShowRect)
   //     target.draw(m_LayoutRect);
}

void UIPanel::ShowRect(bool show) {
    this->m_ShowRect = show;
}
