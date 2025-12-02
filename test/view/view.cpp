#include "view.h"

#include <string>
#include <format>

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/AudioSignals.h"

TestView::TestView(AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr) : Base::View(typeid(TestView), assetMgr, settingsMgr, audioMgr), m_Button(), m_Play(), m_Pause(), m_FPSDeltaTimeText(), m_ProgressBar(0, 100) {
	auto fontHandle = b_mAssetMgr->Get<Font>(std::string("main-font"));
	m_Font = fontHandle.Get();

	m_Button.SetFont(*m_Font);
	m_Play.SetFont(*m_Font);
	m_Pause.SetFont(*m_Font);
	m_Stop.SetFont(*m_Font);

	m_Play.SetText("Play");
	m_Stop.SetText("Stop");
	m_Pause.SetText("Pause");

	// auto musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));

	m_Button.SetOnLClick([this]() { progress = 0; m_ProgressBar.UpdateProgress(progress); m_ProgressBar.Update(0); });
	m_Play.SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)))
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
		else
			musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));

		SignalManager::GetInstance()->Send(std::make_shared<PlayAudioStreamSignal>(musicHandle)); 
	});
	m_Stop.SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)))
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
		else
			musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));

		SignalManager::GetInstance()->Send(std::make_shared<StopAudioStreamSignal>(musicHandle)); 
	});
	m_Pause.SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<PauseAudioStreamSignal>("test-music")); });
}

void TestView::OnDraw(sf::RenderWindow* window) {
	auto& font = m_Font->GetSFMLFont();

	auto text = sf::Text(*font, m_FPSDeltaTimeText, 15);
	text.setPosition({ 0, 0 });

	auto count = sf::Text(*font, std::format("{}", m_ProgressBar.GetProgress()), 15);
	count.setPosition({ 800 - count.getLocalBounds().size.x, 0});

	window->draw(m_Button);
	window->draw(m_Play);
	window->draw(m_Stop);
	window->draw(m_Pause);
	window->draw(m_ProgressBar);
	window->draw(text);
	window->draw(count);
}

void TestView::OnSFMLEvent(std::optional<sf::Event> event) {
	if (auto data = event->getIf<sf::Event::MouseMoved>()) {
		m_Button.OnMouseMove(data->position);
		m_Play.OnMouseMove(data->position);
		m_Stop.OnMouseMove(data->position);
		m_Pause.OnMouseMove(data->position);
	}
	else if (auto data = event->getIf<sf::Event::MouseButtonPressed>()) {
		m_Button.OnMousePressed(data->button, data->position);
		m_Play.OnMousePressed(data->button, data->position);
		m_Stop.OnMousePressed(data->button, data->position);
		m_Pause.OnMousePressed(data->button, data->position);
	}
	else if (auto data = event->getIf<sf::Event::MouseButtonReleased>()) {
		m_Button.OnMouseReleased(data->button, data->position);
		m_Play.OnMouseReleased(data->button, data->position);
		m_Stop.OnMouseReleased(data->button, data->position);
		m_Pause.OnMouseReleased(data->button, data->position);
	}
	else if (auto data = event->getIf<sf::Event::Resized>()) {

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

	m_Button.SetPosition({ 5, 100 });
	m_Button.SetSize({ 80, 30 });
	m_Button.Update(dt);

	m_Play.SetPosition({ 5 , 5 + m_Button.GetPosition().y + m_Button.GetSize().y });
	m_Play.SetSize({ 80, 30 });
	m_Play.Update(dt);

	m_Stop.SetPosition({ 5 + m_Play.GetPosition().x + m_Play.GetSize().x, 5 + m_Button.GetPosition().y + m_Button.GetSize().y});
	m_Stop.SetSize({ 80, 30 });
	m_Stop.Update(dt);

	m_Pause.SetPosition({ 5, 5 + m_Play.GetPosition().y + m_Play.GetSize().y });
	m_Pause.SetSize({ 80, 30 });
	m_Pause.Update(dt);

	m_ProgressBar.SetPosition({ 400 - (m_ProgressBar.GetSize().x / 2), 0});
	m_ProgressBar.SetSize({ 725, 5 });
	m_ProgressBar.Update(dt);
}

void TestView::OnExit() {

}
