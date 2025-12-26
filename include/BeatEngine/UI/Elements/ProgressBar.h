#pragma once

#include "BeatEngine/UI/UIElement.h"

namespace UI {
	class ProgressBar : public UIElement {
	private:
		sf::Color m_InnerColor = sf::Color(255, 255, 255, 255);
		sf::Color m_BackColor = sf::Color(155, 155, 155, 255);

		sf::RectangleShape m_InnerRect;

		float m_Progress;
		float m_MaxValue;

		float m_Percentage = 0;
	public:
		ProgressBar(float current, float max);
		~ProgressBar() override = default;

		void OnUpdate(float dt) override;
		void UpdateProgress(float progress);
        void SetMaxValue(float max);

		float GetProgress() const;
		float GetMaxValue() const;
		float GetPercentage() const;

		void SetInnerColor(sf::Color color);
		void SetBackColor(sf::Color color);

		void OnDraw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;
	};
}
