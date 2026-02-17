#pragma once

#include "BeatEngine/UI/UIClickeable.h"

#include "BeatEngine/Asset/Font.h"

#include <string>
namespace UI {
	class Button : public UIClickeable {
	private:
		std::string m_Text = "Button";
		Font m_Font;
		float m_FontSize = 30;

		// sf::Text m_SFMLText;
		//
		// sf::Color m_NormalColor		= sf::Color::White;
		// sf::Color m_HoverColor		= sf::Color(100, 100, 100, 255);
		// sf::Color m_ActiveColor		= sf::Color(150, 150, 150, 255);
		// sf::Color m_TextNormalColor	= sf::Color::Black;
		// sf::Color m_TextActiveColor = sf::Color::Black;
		// sf::Color m_TextHoverColor = sf::Color::Black;
		//
		// sf::Color m_Color = m_NormalColor;
		// sf::Color m_TextColor = m_TextNormalColor;
	public:
		Button() : Button(Font(), "Button") {};
		// Button(sf::Font font, std::string text, float fontSize = 30);
        Button(Font font, std::string text, float fontSize = 30);

		void SetText(const std::string& text);
		// void SetColors(sf::Color normalColor, sf::Color hoverColor, sf::Color activeColor, sf::Color textColor, sf::Color textActiveColor);
		void SetFontSize(float size);
		// void SetFont(sf::Font font);
		void SetFont(Font font);

		std::string GetText();
		float GetFontSize() const;

		void OnUpdate(float dt) override;

		void OnDraw(/*sf::RenderTarget& target, sf::RenderStates states*/) const override;
	};
}
