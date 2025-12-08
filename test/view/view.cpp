#include "view.h"

#include <string>
#include <format>

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/AudioSignals.h"

TestView::TestView(AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr) 
	: Base::View(typeid(TestView), assetMgr, settingsMgr, audioMgr, uiMgr), m_Button(), m_FPSDeltaTimeText(), m_ProgressBar(0, 200) {
	auto fontHandle = b_mAssetMgr->Get<Font>(std::string("main-font"));
	m_Font = fontHandle.Get();

	m_Button.SetFont(*m_Font);

	auto playButton = m_Button.AddChild<UI::Button>("play");
	auto stopButton = m_Button.AddChild<UI::Button>("stop");
	auto pauseButton = m_Button.AddChild<UI::Button>("pause");

	playButton->SetFont(*m_Font);
	playButton->SetText("Play");
	stopButton->SetFont(*m_Font);
	stopButton->SetText("Stop");
	pauseButton->SetFont(*m_Font);
	pauseButton->SetText("Pause");

	// auto musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));

	m_Button.SetOnLClick([this]() { progress = 0; m_ProgressBar.UpdateProgress(progress); m_ProgressBar.Update(0); });
	playButton->SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)))
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
		else
			musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));

		SignalManager::GetInstance()->Send(std::make_shared<PlayAudioStreamSignal>(musicHandle)); 
	});
	stopButton->SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)))
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
		else
				musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));

		SignalManager::GetInstance()->Send(std::make_shared<StopAudioStreamSignal>(musicHandle)); 
	});
	pauseButton->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<PauseAudioStreamSignal>("test-music")); });
}

void TestView::OnDraw(sf::RenderWindow* window) {
	auto& font = m_Font->GetSFMLFont();

	auto text = sf::Text(*font, m_FPSDeltaTimeText, 15);
	text.setPosition({ 0, 0 });

	auto count = sf::Text(*font, std::format("{}", m_ProgressBar.GetProgress()), 15);
	count.setPosition({ 800 - count.getLocalBounds().size.x, 0});

	auto percentage = sf::Text(*font, std::format("{:.0f}%", m_ProgressBar.GetPercentage() * 100), 15);
	percentage.setPosition({ 800 - percentage.getLocalBounds().size.x, count.getPosition().y + count.getLocalBounds().size.y + 1 });

	window->draw(m_Button);

	window->draw(m_ProgressBar);
	window->draw(text);
	window->draw(percentage);
	window->draw(count);
}

void TestView::OnSFMLEvent(std::optional<sf::Event> event) {
	m_Button.OnSFMLEvent(event);

	if (auto data = event->getIf<sf::Event::Resized>()) {

	}
}

void TestView::OnUpdate(float dt) {
	m_Timer += dt;
	

	if (m_Timer >= 0.1) {
		m_Timer = 0;
		progress += 0.5;
	}
	if (progress <= m_ProgressBar.GetMaxValue())
		m_ProgressBar.UpdateProgress(progress);

	m_FPSDeltaTimeText = std::format("dt {:.3f}\nFPS {:.2f}", dt, 1 / dt);

	m_Button.SetPosition({ 5, 100 });
	m_Button.SetSize({ 80, 30 });
	m_Button.Update(dt);

	auto playBtn = m_Button.GetChild<UI::Button>("play"); 
	playBtn->SetPosition({ 5 , 5 + m_Button.GetPosition().y + m_Button.GetSize().y });
	playBtn->SetSize({ 80, 30 });
	playBtn->Update(dt);

	auto stopBtn = m_Button.GetChild<UI::Button>("stop");
	stopBtn->SetPosition({ 5 + playBtn->GetPosition().x + playBtn->GetSize().x, 5 + m_Button.GetPosition().y + m_Button.GetSize().y});
	stopBtn->SetSize({ 80, 30 });
	stopBtn->Update(dt);

	auto pauseBtn = m_Button.GetChild<UI::Button>("pause");
	pauseBtn->SetPosition({ 5, 5 + playBtn->GetPosition().y + playBtn->GetSize().y });
	pauseBtn->SetSize({ 80, 30 });
	pauseBtn->Update(dt);

	m_ProgressBar.SetPosition({ 400 - (m_ProgressBar.GetSize().x / 2), 0});
	m_ProgressBar.SetSize({ 725, 5 });
	m_ProgressBar.Update(dt);
}

void TestView::OnExit() {

}
