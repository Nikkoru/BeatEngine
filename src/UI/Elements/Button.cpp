#include "BeatEngine/UI/Elements/Button.h"
// #include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/UI/Alignment.h"

UI::Button::Button(Font font, std::string text, float fontSize) : UIClickeable(typeid(Button)), m_Font(font), m_Text(text), m_FontSize(fontSize)/*, m_SFMLText(font, text, m_FontSize)*/ {
	SetOnHover([this]() {
		// m_Color = m_HoverColor;
		// m_TextColor = m_TextHoverColor;
	});
	SetOnUnHover([this]() {
		// m_Color = m_NormalColor;
		// m_TextColor = m_TextNormalColor;
	});
	SetOnActive([this]() {
		// m_Color = m_ActiveColor;
		// m_TextColor = m_TextActiveColor;
	});
	SetOnDeactive([this]() {
		if (m_Hovered) {
			// m_Color = m_HoverColor;
			// m_TextColor = m_TextHoverColor;
		}
		else {
			// m_Color = m_NormalColor;
			// m_TextColor = m_TextNormalColor;
		}
	});
    SetOnHide([this]() {
        if (m_Hovered) {
            // SignalManager::GetInstance()->Send(std::make_shared<GameChangeCursorSignal>(sf::Cursor::Type::Arrow));
        }
        
        m_Hovered = false;
        OnUnHover();
    });
} 
void UI::Button::SetText(const std::string& text) {
	this->m_Text = text;
}

// void UI::Button::SetColors(sf::Color normalColor, sf::Color hoverColor, sf::Color activeColor, sf::Color textColor, sf::Color textActiveColor) {
// 	this->m_ActiveColor = activeColor;
// 	this->m_HoverColor = hoverColor;
// 	this->m_ActiveColor = activeColor;
// 	this->m_TextColor = textColor;
// 	this->m_TextActiveColor = textActiveColor;
// }

void UI::Button::SetFontSize(float size) {
	this->m_FontSize = size;
}

// void UI::Button::SetFont(sf::Font font) {
// 	this->m_Font = font;
// 	m_SFMLText.setFont(font);
// }
//
void UI::Button::SetFont(Font font) {
	this->m_Font = font;
	// m_SFMLText.setFont(*font.GetSFMLFont());
}

std::string UI::Button::GetText() {
	return this->m_Text;
}

float UI::Button::GetFontSize() const {
	return this->m_FontSize;
}

void UI::Button::OnUpdate(float dt) {

	// m_SFMLText = sf::Text(m_Font, m_Text, m_FontSize);
    
    float x = 0, y = 0;
    switch (m_HAlignment) {
    case UIAlignmentH::Left:
        x = m_Position.X;
        break;
    case UIAlignmentH::Right:
        x = m_Position.X;
        break;
    case UIAlignmentH::Center:
        x = m_Position.X / 2 /*- m_SFMLText.getLocalBounds().size.x / 2*/;
    }

    switch (m_VAlignment) {
    case UIAlignmentV::Down:
        y = m_Position.Y;
        break;
    case UIAlignmentV::Up:
        y = m_Position.Y;
        break;
    case UIAlignmentV::Center:
        x = m_Position.Y / 2 /*- m_SFMLText.getLocalBounds().size.y / 2*/;
    }
    
	// m_SFMLText.setPosition({ x, y });
	// m_SFMLText.setFillColor(m_TextColor);

	if (!m_Textures.empty()) {
		// auto& texture = m_Textures.begin()->second.GetSFMLTexture();
		// m_LayoutRect.setTexture(texture.get());
	}

// 	m_LayoutRect.setFillColor(m_Color);
// 	m_LayoutRect.setSize(m_Size);
// 	m_LayoutRect.setPosition(m_Position);
}

void UI::Button::OnDraw(/*sf::RenderTarget& target, sf::RenderStates states*/) const {
// 	target.draw(m_LayoutRect);
// 	target.draw(m_SFMLText);
}
