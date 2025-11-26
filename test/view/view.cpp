#include "view.h"

#include <string>
#include <format>

#include "BeatEngine/Asset/Font.h"

TestView::TestView(AssetManager* assetMgr) : Base::View(assetMgr, typeid(TestView)), m_Button(), m_FPSDeltaTimeText(), m_ProgressBar(0, 100) {
	auto handle = b_mAssetMgr->Get<Font>(std::string("main-font"));
	m_Font = handle.Get();
	m_Button.SetFont(*m_Font);

	m_Button.SetOnLClick([this]() { progress = 0; m_ProgressBar.Update(0); });
}
void TestView::OnDraw(sf::RenderWindow* window) {
	auto& font = m_Font->GetSFMLFont();

	auto text = sf::Text(*font, m_FPSDeltaTimeText, 15);
	text.setPosition({ 0, 0 });

	auto count = sf::Text(*font, std::format("{}", m_ProgressBar.GetProgress()), 25);
	count.setPosition({ 400 + m_ProgressBar.GetSize().x, 15 });

	window->draw(m_Button);
	window->draw(m_ProgressBar);
	window->draw(text);
	window->draw(count);
}

void TestView::OnSFMLEvent(std::optional<sf::Event> event) {
	if (auto data = event->getIf<sf::Event::MouseMoved>()) {
		m_Button.OnMouseMove(data->position);
	}
	else if (auto data = event->getIf<sf::Event::MouseButtonPressed>()) {
		m_Button.OnMousePressed(data->button, data->position);
	}
	else if (auto data = event->getIf<sf::Event::MouseButtonReleased>()) {
		m_Button.OnMouseReleased(data->button, data->position);
	}
}

void TestView::OnUpdate(float dt) {
	m_Timer += dt;
	
	if (m_Timer >= 0.25) {
		m_Timer = 0;
		progress += 0.5;

		if (progress <= 100)
			m_ProgressBar.UpdateProgress(progress);
	}	

	m_FPSDeltaTimeText = std::format("dt {:.2f}\nFPS {:.2f}", dt, 1 / dt);

	m_Button.SetPosition({ 100, 100 });
	m_Button.SetSize({ 80, 30 });
	m_Button.Update(dt);

	m_ProgressBar.SetPosition({ 400 - (m_ProgressBar.GetSize().x / 2), 0});
	m_ProgressBar.SetSize({ 800, 5 });
	m_ProgressBar.Update(dt);
}

void TestView::OnExit() {

}
