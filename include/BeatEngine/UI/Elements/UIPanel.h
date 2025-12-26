#pragma once

#include "BeatEngine/UI/UIElement.h"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class UIPanel : public UIElement {
private:
    sf::Color m_ColorRect = sf::Color::White;
    bool m_ShowRect = true;
public:
    UIPanel() = default;
    void OnDraw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void OnUpdate(float dt) override {}

    void ShowRect(bool show);
};
