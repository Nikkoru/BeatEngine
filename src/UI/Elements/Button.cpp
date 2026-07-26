#include "BeatEngine/UI/Elements/Button.h"
// #include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/UI/Alignment.h"
#include "imgui.h"

UI::Button::Button(std::shared_ptr<Font> font, std::string text, float fontSize) : UIClickeable(typeid(Button)), m_Text(text), m_Font(font), m_FontSize(fontSize) , m_TextElement(font, text, m_FontSize) {
	SetOnHover([&]() {
		m_Color = m_HoverColor;
		m_TextColor = m_TextHoverColor;
	});
	SetOnUnHover([&]() {
		m_Color = m_NormalColor;
		m_TextColor = m_TextNormalColor;
	});
	SetOnActive([&]() {
		m_Color = m_ActiveColor;
		m_TextColor = m_TextActiveColor;
	});
	SetOnDeactive([&]() {
		if (m_Hovered) {
			m_Color = m_HoverColor;
			m_TextColor = m_TextHoverColor;
		}
		else {
			m_Color = m_NormalColor;
			m_TextColor = m_TextNormalColor;
		}
	});
    SetOnHide([&]() {
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

void UI::Button::SetFontSize(float size) {
	this->m_FontSize = size;
}

void UI::Button::SetFont(std::shared_ptr<Font> font) {
	this->m_Font = font;
	m_TextElement.SetFont(font);
}

std::string UI::Button::GetText() {
	return this->m_Text;
}

float UI::Button::GetFontSize() const {
	return this->m_FontSize;
}

void UI::Button::OnUpdate(float dt) {
    (void)dt;

    if (m_Text != m_TextElement.GetText())
        m_TextElement.SetString(m_Text);
    if (m_FontSize != m_TextElement.GetCharacterSize())
        m_TextElement.SetCharacterSize(m_FontSize);

    Vector2f textPos{};
    switch (m_HAlignment) {
    case UIAlignmentH::Left:
        if (m_TextElement.GetLineAlignment() != TextElement::LineAlignment::Left)
            m_TextElement.SetLineAlignment(TextElement::LineAlignment::Left);
        textPos.X = m_Position.X;
        break;
    case UIAlignmentH::Right:
        if (m_TextElement.GetLineAlignment() != TextElement::LineAlignment::Right)
            m_TextElement.SetLineAlignment(TextElement::LineAlignment::Right);

        textPos.X = m_Position.X + m_Size.X;
        break;
    case UIAlignmentH::Center:
        if (m_TextElement.GetLineAlignment() != TextElement::LineAlignment::Center)
            m_TextElement.SetLineAlignment(TextElement::LineAlignment::Center);
        textPos.X = m_Position.X + m_Size.X / 2.f; 
        break;
    }

    switch (m_VAlignment) {
    case UIAlignmentV::Down:
        textPos.Y = m_Position.Y;
        break;
    case UIAlignmentV::Up:
        textPos.Y = m_Position.Y;
        break;
    case UIAlignmentV::Center:
        textPos.Y = m_Position.Y;
    }
    
    if (textPos != m_TextElement.GetPosition())
        m_TextElement.SetPosition(textPos);
    if (m_TextColor != m_TextElement.GetColor())
        m_TextElement.SetColor(m_TextColor);

	m_LayoutRect.SetColor(m_Color);
	m_LayoutRect.SetSize(m_Size);
	m_LayoutRect.SetPosition(m_Position);
}

void UI::Button::OnDraw(GraphicsManager& mgr, RenderState state) {
	m_LayoutRect.Draw(mgr, state);
	m_TextElement.Draw(mgr, state);
}

void UI::Button::SpecificImGuiDebug() {
    ImGui::Text("Button text: %s", m_Text.c_str());
    ImGui::Text("Font size: %f", m_FontSize);
    m_Hovered ? ImGui::TextColored({ 0, 255, 0, 255 }, "Hovered") : ImGui::TextColored({ 255, 0, 0, 255 }, "Not Hovered");
}
