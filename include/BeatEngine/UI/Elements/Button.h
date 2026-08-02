#pragma once

#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/TextElement.hpp"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/UI/UIClickeable.h"

#include "BeatEngine/Asset/Font.h"

#include <string>
namespace UI {
	class Button : public UIClickeable {
	private:
		std::string m_Text = "Button";
        Base::AssetHandle<Font> m_Font{};
		float m_FontSize = 30;

		TextElement m_TextElement;
        
		RGBColor m_NormalColor		= RGBColor::White();
		RGBColor m_HoverColor		= RGBColor(100, 100, 100, 255);
		RGBColor m_ActiveColor		= RGBColor(150, 150, 150, 255);
		RGBColor m_TextNormalColor	= RGBColor::Black();
		RGBColor m_TextHoverColor = RGBColor::Black();
		RGBColor m_TextActiveColor = RGBColor::Black();

		RGBColor m_Color = m_NormalColor;
		RGBColor m_TextColor = m_TextNormalColor;
	public:
		Button() : Button({}, "Button") {};
        Button(Base::AssetHandle<Font> font, std::string text, float fontSize = 30);

		void SetText(const std::string& text);
        void SetNormalColor(RGBColor color) { m_NormalColor = color; }
        void SetHoverColor(RGBColor color) { m_HoverColor = color; }
        void SetActiveColor(RGBColor color) { m_ActiveColor = color; }
        void SetTextNormalColor(RGBColor color) { m_TextNormalColor = color; }
        void SetTextHoverColor(RGBColor color) { m_TextHoverColor = color; }
        void SetTextActiveColor(RGBColor color) { m_TextActiveColor = color; }

		void SetFontSize(float size);
		void SetFont(const Base::AssetHandle<Font>& font);

		std::string GetText();
		float GetFontSize() const;

		void OnUpdate(float dt) override;

		void OnDraw(GraphicsManager& mgr, RenderState state = RenderState::Default) override;

        void SpecificImGuiDebug() override;
	};
}
