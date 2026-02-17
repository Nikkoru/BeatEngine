#include "BeatEngine/UI/Elements/ProgressBar.h"

#include "BeatEngine/Util/UIHelper.h"

UI::ProgressBar::ProgressBar(float current, float max) : m_Progress(current), m_MaxValue(max) {
	SetSize({ 100, 20 });
	// m_InnerRect.setFillColor(m_InnerColor);
	// m_LayoutRect.setFillColor(m_BackColor);
}

void UI::ProgressBar::OnUpdate(float dt) {
	m_Percentage = (m_Progress / m_MaxValue);

	// m_InnerRect.setPosition(m_Position);
	// m_LayoutRect.setPosition(m_Position);

	auto x = UIHelper::Pertentage2PixelsX(m_Percentage, m_Size);

	// m_InnerRect.setSize({ x, m_Size.y });
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

// void UI::ProgressBar::SetInnerColor(sf::Color color) {
// 	m_InnerColor = color;
// 	m_InnerRect.setFillColor(color);
// }
//
// void UI::ProgressBar::SetBackColor(sf::Color color) {
// 	m_BackColor = color;
// 	m_LayoutRect.setFillColor(color);
// }

void UI::ProgressBar::OnDraw(/*sf::RenderTarget& target, sf::RenderStates states*/) const {
	// target.draw(m_LayoutRect);
	// target.draw(m_InnerRect);
}
