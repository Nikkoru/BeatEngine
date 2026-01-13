#include "view.h"

#include <SFML/Graphics/Text.hpp>
#include <cmath>
#include <memory>
#include <string>
#include <format>

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/AudioSignals.h"
#include "BeatEngine/Events/AudioEvent.h"

TestView::TestView(AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr) 
	: Base::View(typeid(TestView), assetMgr, settingsMgr, audioMgr, uiMgr), m_Button(), m_ProgressBar(0, 200), m_MusicProgressBar(0, 0) {
    m_MusicProgressBar.Hide();

	auto fontHandle = b_mAssetMgr->Get<Font>(std::string("main-font"));
	m_Font = fontHandle.Get();

	m_Button.SetFont(*m_Font);

	auto playButton = m_Button.AddChild<UI::Button>("play", *m_Font, "Play");
	auto stopButton = m_Button.AddChild<UI::Button>("stop", *m_Font, "Stop");
	auto pauseButton = m_Button.AddChild<UI::Button>("pause", *m_Font, "Pause");

    // playButton->SetVAlignment(UIAlignmentV::Center);
    // playButton->SetHAlignment(UIAlignmentH::Center);

    // stopButton->SetVAlignment(UIAlignmentV::Center);
    // stopButton->SetHAlignment(UIAlignmentH::Center);

    // pauseButton->SetVAlignment(UIAlignmentV::Center);
    // pauseButton->SetHAlignment(UIAlignmentH::Center);

	m_Button.SetOnLClick([this]() { progress = 0; m_ProgressBar.UpdateProgress(progress); m_ProgressBar.Update(0); });
	playButton->SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)))
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
		else
			musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));
        
        b_mAudioMgr->PlayStream(musicHandle.Get());
	});
	stopButton->SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)) && b_mAudioMgr->IsStreamPlaying("test-music")) {
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
            b_mAudioMgr->StopStream(musicHandle.Get());
        }

	});
	pauseButton->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<PauseAudioStreamSignal>("test-music")); });

    EventManager::GetInstance()->SubscribeView<AudioStreamStopedEvent>(typeid(TestView), [this](std::shared_ptr<Base::Event> event) {
        auto audioEvent = std::static_pointer_cast<AudioStreamStopedEvent>(event);

        if (audioEvent->Name == "test-music") {
            m_MusicProgressBar.Hide();
        }
    });

    EventManager::GetInstance()->SubscribeView<AudioStreamStartedEvent>(typeid(TestView), [this](std::shared_ptr<Base::Event> event) {
        auto audioEvent = std::static_pointer_cast<AudioStreamStartedEvent>(event);

        if (audioEvent->Name == "test-music") {
            auto handle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
            m_MusicProgressBar.Show();

            auto musicMetadata = handle.Get()->GetMetadata();

            m_MusicTitleText = musicMetadata.Title.toWString();
            m_MusicAlbumText = musicMetadata.Artist.toWString();
            m_MusicTrackNumText = std::to_wstring(musicMetadata.TrackNum); 
            m_MusicYearText = std::to_wstring(musicMetadata.Year);

            m_MusicProgressBar.SetMaxValue(handle.Get()->GetTotalSeconds());
        }
    });
}

void TestView::OnDraw(sf::RenderWindow* window) {
	auto font = m_Font->GetSFMLFont();
    
    auto musicTitle = sf::Text(*font, m_MusicTitleText, 15);
    musicTitle.setPosition({ 400 - (musicTitle.getGlobalBounds().size.x / 2), 585 - musicTitle.getGlobalBounds().size.y });

    auto musicAlbum = sf::Text(*font, m_MusicAlbumText, 15);
    musicAlbum.setPosition({ 400 - (musicAlbum.getGlobalBounds().size.x / 2), musicTitle.getPosition().y - musicAlbum.getGlobalBounds().size.y - 1 });

    auto musicTrackNum = sf::Text(*font, m_MusicTrackNumText, 15);
    musicTrackNum.setPosition({ 400 - (musicTrackNum.getGlobalBounds().size.x / 2), musicAlbum.getPosition().y - musicTrackNum.getGlobalBounds().size.y - 1 });

    auto musicYear = sf::Text(*font, m_MusicYearText, 15);
    musicYear.setPosition({ 400 - (musicYear.getGlobalBounds().size.x / 2), musicTrackNum.getPosition().y - musicYear.getGlobalBounds().size.y - 1 });

	auto count = sf::Text(*font, std::format("{}", m_ProgressBar.GetProgress()), 15);
	count.setPosition({ 800 - count.getLocalBounds().size.x, 0});

	auto percentage = sf::Text(*font, std::format("{:.0f}%", m_ProgressBar.GetPercentage() * 100), 15);
	percentage.setPosition({ 800 - percentage.getLocalBounds().size.x, count.getPosition().y + count.getLocalBounds().size.y + 1 });

    auto countMusic = sf::Text(*font, std::format("{}", m_MusicProgressBar.GetProgress()), 15);
    countMusic.setPosition({ 400 - countMusic.getGlobalBounds().size.x / 2, 300 });

    auto maxMusic = sf::Text(*font, std::format("{}", m_MusicProgressBar.GetMaxValue()), 15);
    maxMusic.setPosition({ 400 - maxMusic.getGlobalBounds().size.x / 2, countMusic.getPosition().y + maxMusic.getGlobalBounds().size.y - 1 });

	window->draw(m_Button);

	window->draw(m_ProgressBar);
    window->draw(m_MusicProgressBar);

    if (m_MusicProgressBar.IsVisible()) {
        window->draw(musicTitle);
        window->draw(musicAlbum);
        window->draw(musicTrackNum);
        window->draw(musicYear);

        window->draw(countMusic);
        window->draw(maxMusic);
    }

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

    if (m_MusicProgressBar.IsVisible()) {
        m_MusicProgressBar.UpdateProgress(std::floor(b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView)).Get()->GetTranscurredSeconds()));
    }

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

    m_MusicProgressBar.SetPosition({ 400 - (m_MusicProgressBar.GetSize().x / 2), 590 });
    m_MusicProgressBar.SetSize({ 300, 5 });
    m_MusicProgressBar.Update(dt);
}

void TestView::OnExit() {

}
