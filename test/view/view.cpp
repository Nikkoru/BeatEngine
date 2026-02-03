#include "view.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <cmath>
#include <memory>
#include <string>
#include <format>

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/AudioSignals.h"
#include "BeatEngine/Events/AudioEvent.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Signals/ViewSignals.h"
#include "BeatEngine/UI/Elements/Button.h"
#include "BeatEngine/UI/Elements/ProgressBar.h"
#include "gameView.h"

TestView::TestView(GameContext* context, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr) 
	: Base::View(typeid(TestView), context, assetMgr, settingsMgr, audioMgr, uiMgr) {
    auto windowSize = b_mContext->WindowSize;
    
    m_HUD = uiMgr->AddLayer("mainViewUI");

    auto button = m_HUD->SetRootElement<UI::Button>();
    auto progressBar = button->AddChild<UI::ProgressBar>("prog", 0, 200);

	auto fontHandle = b_mAssetMgr->Get<Font>(std::string("main-font"));
	m_Font = fontHandle.Get();

	button->SetFont(*m_Font);
    button->SetPosition({ 5, 100 });
	button->SetSize({ 80, 30 });

	auto playBtn = button->AddChild<UI::Button>("playBtn", *m_Font, "Play");
	playBtn->SetSize({ 80, 30 });
    playBtn->SetPosition({ 5 , 5 + button->GetPosition().y + button->GetSize().y });

	auto stopBtn = button->AddChild<UI::Button>("stopBtn", *m_Font, "Stop");
	stopBtn->SetSize({ 80, 30 });
    stopBtn->SetPosition({ 5 + playBtn->GetPosition().x + playBtn->GetSize().x, 5 + button->GetPosition().y + button->GetSize().y});

	auto pauseBtn = button->AddChild<UI::Button>("pauseBtn", *m_Font, "Pause");
	pauseBtn->SetSize({ 80, 30 });
    pauseBtn->SetPosition({ 5, 5 + playBtn->GetPosition().y + playBtn->GetSize().y });

    auto gameBtn = button->AddChild<UI::Button>("gameBtn", *m_Font, "Game");
    gameBtn->SetSize({ 80, 30 });
    gameBtn->SetPosition({ static_cast<float>(windowSize.x / 2) - static_cast<float>(gameBtn->GetSize().x / 2), 100 });

    playBtn->SetVAlignment(UIAlignmentV::Center);
    playBtn->SetHAlignment(UIAlignmentH::Center);

    stopBtn->SetVAlignment(UIAlignmentV::Center);
    stopBtn->SetHAlignment(UIAlignmentH::Center);

    pauseBtn->SetVAlignment(UIAlignmentV::Center);
    pauseBtn->SetHAlignment(UIAlignmentH::Center);

	button->SetOnLClick([this, progressBar]() { progress = 0; progressBar->UpdateProgress(progress); progressBar->Update(0); });
	playBtn->SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)))
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
		else
			musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));
        
        b_mAudioMgr->PlayStream(musicHandle.Get());
	});
	stopBtn->SetOnLClick([this]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mAssetMgr->Has("test-music", typeid(TestView)) && b_mAudioMgr->IsStreamPlaying("test-music")) {
			musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
            b_mAudioMgr->StopStream(musicHandle.Get());
        }

	});
	pauseBtn->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<PauseAudioStreamSignal>("test-music")); });
    gameBtn->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<ViewPushSignal>(typeid(GameView))); });

    EventManager::GetInstance()->SubscribeView<AudioStreamStopedEvent>(typeid(TestView), [this, button](std::shared_ptr<Base::Event> event) {
        auto audioEvent = std::static_pointer_cast<AudioStreamStopedEvent>(event);

        if (audioEvent->Name == "test-music" && button->HasChild("musicProg")) {
            button->RemoveChild("musicProg");
        }
    });

    EventManager::GetInstance()->SubscribeView<AudioStreamStartedEvent>(typeid(TestView), [this, button](std::shared_ptr<Base::Event> event) {
        auto audioEvent = std::static_pointer_cast<AudioStreamStartedEvent>(event);

        if (audioEvent->Name == "test-music") {
            auto handle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
            auto musicProgressBar = button->AddChild<UI::ProgressBar>("musicProg", 0, handle.Get()->GetTotalSeconds());

            auto musicMetadata = handle.Get()->GetMetadata();

            m_MusicTitleText = musicMetadata.Title.toWString();
            m_MusicAlbumText = musicMetadata.Artist.toWString();
            m_MusicTrackNumText = std::to_wstring(musicMetadata.TrackNum); 
            m_MusicYearText = std::to_wstring(musicMetadata.Year);
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
    
    auto progressBar = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::ProgressBar>("prog");

	auto count = sf::Text(*font, std::format("{}", progressBar->GetProgress()), 15);
	count.setPosition({ 800 - count.getLocalBounds().size.x, 0});

	auto percentage = sf::Text(*font, std::format("{:.0f}%", progressBar->GetPercentage() * 100), 15);
	percentage.setPosition({ 800 - percentage.getLocalBounds().size.x, count.getPosition().y + count.getLocalBounds().size.y + 1 });


	// window->draw(*m_HUD);

    if (m_HUD->GetRootElement<UI::Button>()->HasChild("musicProg")) {
        auto musicProgressBar = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::ProgressBar>("musicProg");

        auto countMusic = sf::Text(*font, std::format("{}", musicProgressBar->GetProgress()), 15);
        countMusic.setPosition({ 400 - countMusic.getGlobalBounds().size.x / 2, 300 });

        auto maxMusic = sf::Text(*font, std::format("{}", musicProgressBar->GetMaxValue()), 15);
        maxMusic.setPosition({ 400 - maxMusic.getGlobalBounds().size.x / 2, countMusic.getPosition().y + maxMusic.getGlobalBounds().size.y - 1 });

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
	m_HUD->OnSFMLEvent(event);

    if (auto data = event->getIf<sf::Event::KeyPressed>()) {
        if (data->scancode == sf::Keyboard::Scan::Escape)
            SignalManager::GetInstance()->Send(std::make_shared<GameExitSignal>());
    }
    if (event->is<sf::Event::Resized>()) {
        auto windowSize = b_mContext->WindowSize;
        auto btn = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::Button>("gameBtn");
        btn->SetPosition({ static_cast<float>(windowSize.x / 2) - static_cast<float>(btn->GetSize().x / 2), 100 });
    }
}

void TestView::OnUpdate(float dt) {
    auto progressBar = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::ProgressBar>("prog");

	m_Timer += dt;

	if (m_Timer >= 0.1) {
		m_Timer = 0;
		progress += 0.5;
	}
	if (progress <= progressBar->GetMaxValue())
		progressBar->UpdateProgress(progress);

    if (m_HUD->GetRootElement<UI::Button>()->HasChild("musicProg")) {
        auto musicProgressBar = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::ProgressBar>("musicProg");
        musicProgressBar->UpdateProgress(std::floor(b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView)).Get()->GetTranscurredSeconds()));

        musicProgressBar->SetPosition({ 400 - (musicProgressBar->GetSize().x / 2), 590 });
        musicProgressBar->SetSize({ 300, 5 });
        musicProgressBar->Update(dt);
    }

	progressBar->SetPosition({ 400 - (progressBar->GetSize().x / 2), 0});
	progressBar->SetSize({ 725, 5 });
	progressBar->Update(dt);

}

void TestView::OnExit() {
    Logger::AddInfo(typeid(TestView), "OnExit() called");
}

void TestView::OnSuspend() {
    Logger::AddInfo(typeid(TestView), "OnSuspend() called");

    m_HUD->SetVisible(false);
}

void TestView::OnResume() {
    Logger::AddInfo(typeid(TestView), "OnResume() called");

    m_HUD->SetVisible(true);
}
