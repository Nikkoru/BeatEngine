#include "BeatEngine/UI/Elements/ProgressBar.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/UI/UIElement.h"
#include "BeatEngine/Util/UIHelper.h"

UI::ProgressBar::ProgressBar(float current, float max) : UIElement(typeid(ProgressBar)), m_Progress(current), m_MaxValue(max) {
	SetSize({ 100, 20 });
	m_InnerRect.SetColor(m_InnerColor);
	m_LayoutRect.SetColor(m_BackColor);
}

void UI::ProgressBar::OnUpdate(float dt) {
    (void)dt;
	m_Percentage = (m_Progress / m_MaxValue);

	m_InnerRect.SetPosition(m_Position);
	m_LayoutRect.SetPosition(m_Position);

	auto x = UIHelper::Pertentage2PixelsX(m_Percentage, m_Size);

	m_InnerRect.SetSize({ x, m_Size.Y });
}

void UI::ProgressBar::UpdateProgress(float progress) {
	this->m_Progress = progress;
}

void UI::ProgressBar::SetMaxValue(float max) {
    this->m_MaxValue = max;
}

float UI::ProgressBar::GetProgress() const {
	return m_Progress;
}

float UI::ProgressBar::GetMaxValue() const {
	return m_MaxValue;
}

float UI::ProgressBar::GetPercentage() const {
	return m_Percentage;
}

void UI::ProgressBar::SetInnerColor(RGBColor color) {
	m_InnerColor = color;
	m_InnerRect.SetColor(color);
}

void UI::ProgressBar::SetBackColor(RGBColor color) {
	m_BackColor = color;
	m_LayoutRect.SetColor(color);
}

void UI::ProgressBar::OnDraw(GraphicsManager& mgr, RenderState state) {
	m_LayoutRect.Draw(mgr, state);
	m_InnerRect.Draw(mgr, state);
}

void UI::ProgressBar::OnUninitGraphics(GraphicsManager& mgr) {
    m_LayoutRect.UninitGraphics(mgr);
    m_InnerRect.UninitGraphics(mgr);
}

void UI::ProgressBar::SpecificImGuiDebug() {
    ImGui::Text("ProgressBar Progress: %f", m_Progress);
    ImGui::Text("ProgressBar MaxValue: %f", m_MaxValue);
}
