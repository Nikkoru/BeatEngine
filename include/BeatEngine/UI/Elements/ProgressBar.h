#pragma once

#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/UI/UIElement.h"

namespace UI {
	class ProgressBar : public UIElement {
	private:
		RGBColor m_InnerColor{ 255, 255, 255 };
		RGBColor m_BackColor{ 155, 155, 155 };

		RectShape m_InnerRect;

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

		void SetInnerColor(RGBColor color);
		void SetBackColor(RGBColor color);

		void OnDraw(GraphicsManager& mgr) override;
		void OnUninitGraphics(GraphicsManager& mgr) override;

        void SpecificImGuiDebug() override;
	};
}
