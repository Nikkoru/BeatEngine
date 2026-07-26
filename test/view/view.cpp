#include "view.h"

#include <cmath>
#include <memory>
#include <string>

// #include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

// #include "BeatEngine/Base/Event.h"
// #include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/GraphicsManager.h"
// #include "BeatEngine/Manager/SignalManager.h"
// #include "BeatEngine/Signals/AudioSignals.h"
#include "BeatEngine/Events/AudioEvent.h"
// #include "BeatEngine/Signals/GameSignals.h"
// #include "BeatEngine/Signals/ViewSignals.h"
#include "BeatEngine/Signals/AudioSignals.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Signals/ViewSignals.h"
#include "BeatEngine/UI/Elements/Button.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/UI/Elements/ProgressBar.h"
#include "BeatEngine/UI/UILayer.h"
#include "gameView.h"
// #include "gameView.h"

#include <imgui.h>

TestView::TestView(GameContext* context, GameState* state) 
	: Base::View(typeid(TestView), context, state) {

	// auto playBtn = button->AddChild<UI::Button>("playBtn", *m_Font, "Play");
	// playBtn->SetSize({ 80, 30 });
 //    playBtn->SetPosition({ 5 , 5 + button->GetPosition().Y + button->GetSize().Y });
	//
	// auto stopBtn = button->AddChild<UI::Button>("stopBtn", *m_Font, "Stop");
	// stopBtn->SetSize({ 80, 30 });
 //    stopBtn->SetPosition({ 5 + playBtn->GetPosition().X + playBtn->GetSize().X, 5 + button->GetPosition().Y + button->GetSize().Y });
	//
	// auto pauseBtn = button->AddChild<UI::Button>("pauseBtn", *m_Font, "Pause");
	// pauseBtn->SetSize({ 80, 30 });
 //    pauseBtn->SetPosition({ 5, 5 + playBtn->GetPosition().Y + playBtn->GetSize().Y });
	//
 //    auto gameBtn = button->AddChild<UI::Button>("gameBtn", *m_Font, "Game");
 //    gameBtn->SetSize({ 80, 30 });
 //    gameBtn->SetPosition({ static_cast<float>(windowSize.Y / 2) - static_cast<float>(gameBtn->GetSize().Y / 2), 100 });
	//
 //    playBtn->SetVAlignment(UIAlignmentV::Center);
 //    playBtn->SetHAlignment(UIAlignmentH::Center);
	//
 //    stopBtn->SetVAlignment(UIAlignmentV::Center);
 //    stopBtn->SetHAlignment(UIAlignmentH::Center);
	//
 //    pauseBtn->SetVAlignment(UIAlignmentV::Center);
 //    pauseBtn->SetHAlignment(UIAlignmentH::Center);
	//
	// button->SetOnLClick([this, progressBar]() { progress = 0; progressBar->UpdateProgress(progress); progressBar->Update(0); });
	// playBtn->SetOnLClick([this]() {
	// 	Base::AssetHandle<AudioStream> musicHandle;
	//
	// 	if (b_mAssetMgr->Has("test-music", typeid(TestView)))
	// 		musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
	// 	else
	// 		musicHandle = b_mAssetMgr->Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));
 //        
 //        b_mAudioMgr->PlayStream(musicHandle.Get());
	// });
	// stopBtn->SetOnLClick([this]() {
	// 	Base::AssetHandle<AudioStream> musicHandle;
	//
	// 	if (b_mAssetMgr->Has("test-music", typeid(TestView)) && b_mAudioMgr->IsStreamPlaying("test-music")) {
	// 		musicHandle = b_mAssetMgr->Get<AudioStream>("test-music", typeid(TestView));
 //            b_mAudioMgr->StopStream(musicHandle.Get());
 //        }
	//
	// });
	// pauseBtn->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<PauseAudioStreamSignal>("test-music")); });
 //    gameBtn->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<ViewPushSignal>(typeid(GameView))); });
	//

}

void TestView::Init() {
    Vector2f size{ static_cast<float>(b_mContext->WindowSize.X), static_cast<float>(b_mContext->WindowSize.Y) };
    m_Camera.InitOrtho2D(size);
    b_mState->GetGraphicsMgr().SetMainCamera(m_Camera);

    m_Font = b_mState->GetAssetMgr().Get<Font>("main-font").Get(); 

    auto windowSize = b_mContext->WindowSize;
    
    m_HUD = b_mState->GetUIMgr().AddLayer("mainViewUI");
    // m_Text.SetPosition();

    auto button = m_HUD->SetRootElement<UI::Button>();
    auto progressBar = button->AddChild<UI::ProgressBar>("prog", 0, 200);

	auto playBtn = button->AddChild<UI::Button>("playBtn", m_Font, "Play");
	playBtn->SetSize({ 80, 30 });
    playBtn->SetPosition({ 5 , 10 + button->GetPosition().Y + button->GetSize().Y });

	auto stopBtn = button->AddChild<UI::Button>("stopBtn", m_Font, "Stop");
	stopBtn->SetSize({ 80, 30 });
    stopBtn->SetPosition({ 5 + playBtn->GetPosition().X + playBtn->GetSize().X, 10 + button->GetPosition().Y + button->GetSize().Y });

	auto pauseBtn = button->AddChild<UI::Button>("pauseBtn", m_Font, "Pause");
	pauseBtn->SetSize({ 80, 30 });
    pauseBtn->SetPosition({ 5, 10 + playBtn->GetPosition().Y + playBtn->GetSize().Y });

    auto gameBtn = button->AddChild<UI::Button>("gameBtn", m_Font, "Game");
    gameBtn->SetSize({ 80, 30 });
    gameBtn->SetPosition({ static_cast<float>(windowSize.Y / 2) - static_cast<float>(gameBtn->GetSize().Y / 2), 100 });

    playBtn->SetVAlignment(UIAlignmentV::Center);
    playBtn->SetHAlignment(UIAlignmentH::Center);

    stopBtn->SetVAlignment(UIAlignmentV::Center);
    stopBtn->SetHAlignment(UIAlignmentH::Center);

    pauseBtn->SetVAlignment(UIAlignmentV::Center);
    pauseBtn->SetHAlignment(UIAlignmentH::Center);


	button->SetOnLClick([this, progressBar]() { progress = 0; progressBar->UpdateProgress(progress); progressBar->Update(0); });
	playBtn->SetOnLClick([&]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mState->GetAssetMgr().Has("test-music", typeid(TestView)))
			musicHandle = b_mState->GetAssetMgr().Get<AudioStream>("test-music", typeid(TestView));
		else
			musicHandle = b_mState->GetAssetMgr().Load<AudioStream>("assets/music/test-music.mp3", typeid(TestView));
        
        if (musicHandle)
            b_mState->GetAudioMgr().PlayStream(musicHandle.Get());
	});
	stopBtn->SetOnLClick([&]() {
		Base::AssetHandle<AudioStream> musicHandle;

		if (b_mState->GetAssetMgr().Has("test-music", typeid(TestView)) && b_mState->GetAudioMgr().IsStreamPlaying("test-music")) {
			musicHandle = b_mState->GetAssetMgr().Get<AudioStream>("test-music", typeid(TestView));
            b_mState->GetAudioMgr().StopStream(musicHandle.Get());
        }

	});
	pauseBtn->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<PauseAudioStreamSignal>("test-music")); });
    gameBtn->SetOnLClick([]() { SignalManager::GetInstance()->Send(std::make_shared<ViewPushSignal>(typeid(GameView))); });

	// auto fontHandle = b_mAssetMgr->Get<Font>(std::string("main-font"));
	// m_Font = fontHandle.Get();

	// button->SetFont(*m_Font);
    
    auto texture = b_mState->GetAssetMgr().Load<Texture>("assets/textures/texture.png", typeid(TestView));
    auto funnyTexture = b_mState->GetAssetMgr().Load<Texture>("assets/textures/four.jpeg", typeid(TestView));

    m_Shape.SetTexture(texture.Get());
    m_Shape.SetSize(Vector2f{ texture.Get()->GetSize() });
    m_Shape.SetPosition({ size.X / 2 - m_Shape.GetSize().X / 2, size.Y / 2 - m_Shape.GetSize().Y / 2 - 100 });

    m_Shape.SetColor(LinearColor::White());

    m_FunnyShape.SetTexture(funnyTexture.Get());
    m_FunnyShape.SetSize(Vector2f{ funnyTexture.Get()->GetSize() });
    m_FunnyShape.SetPosition({ size.X / 2 - m_FunnyShape.GetSize().X / 2, size.Y / 2 - m_FunnyShape.GetSize().Y / 2 + m_Shape.GetSize().Y + 5 - 100 });

    m_FunnyShape.SetColor(LinearColor::White());

    m_Text = TextElement{ m_Font, "Test", 30 };
    m_Text.SetColor(LinearColor::White());
    m_Text.SetPosition({ size.X / 2 , size.Y / 2 + 5 - 210 });
    m_Text.SetLineAlignment(TextElement::LineAlignment::Center);
    m_Text.SetCharacterSize(50);

    button->SetFont(m_Font);
    button->SetPosition({ 5, 100 });
	button->SetSize({ 80, 30 });

    EventManager::GetInstance()->SubscribeView<EventAudioStreamStoped>(typeid(TestView), [button](std::shared_ptr<Base::Event> event) {
        auto audioEvent = std::static_pointer_cast<EventAudioStreamStoped>(event);

        if (audioEvent->Name == "audio" && button->HasChild("musicProg")) {
            button->RemoveChild("musicProg");
        }
    });

    EventManager::GetInstance()->SubscribeView<EventAudioStreamStarted>(typeid(TestView), [this, button](std::shared_ptr<Base::Event> event) {
        auto audioEvent = std::static_pointer_cast<EventAudioStreamStarted>(event);

        if (audioEvent->Name == "test-music" && b_mState->GetAssetMgr().Has("audio", typeid(TestView))) {
            auto handle = b_mState->GetAssetMgr().Get<AudioStream>("audio", typeid(TestView));
            auto musicProgressBar = button->AddChild<UI::ProgressBar>("musicProg", 0, handle.Get()->GetTotalSeconds());

            auto musicMetadata = handle.Get()->GetMetadata();

            m_MusicTitleText = musicMetadata.Title.toWString();
            m_MusicAlbumText = musicMetadata.Artist.toWString();
            m_MusicTrackNumText = std::to_wstring(musicMetadata.TrackNum); 
            m_MusicYearText = std::to_wstring(musicMetadata.Year);
        }
    });
}

void TestView::OnDraw(GraphicsManager& mgr) {
	// auto font = m_Font->GetSFMLFont();
 //    
 //    auto musicTitle = sf::Text(*font, m_MusicTitleText, 15);
 //    musicTitle.setPosition({ 400 - (musicTitle.getGlobalBounds().size.x / 2), 585 - musicTitle.getGlobalBounds().size.y });
	//
 //    auto musicAlbum = sf::Text(*font, m_MusicAlbumText, 15);
 //    musicAlbum.setPosition({ 400 - (musicAlbum.getGlobalBounds().size.x / 2), musicTitle.getPosition().y - musicAlbum.getGlobalBounds().size.y - 1 });
	//
 //    auto musicTrackNum = sf::Text(*font, m_MusicTrackNumText, 15);
 //    musicTrackNum.setPosition({ 400 - (musicTrackNum.getGlobalBounds().size.x / 2), musicAlbum.getPosition().y - musicTrackNum.getGlobalBounds().size.y - 1 });
	//
 //    auto musicYear = sf::Text(*font, m_MusicYearText, 15);
 //    musicYear.setPosition({ 400 - (musicYear.getGlobalBounds().size.x / 2), musicTrackNum.getPosition().y - musicYear.getGlobalBounds().size.y - 1 });
 //    
    // auto progressBar = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::ProgressBar>("prog");
	//
	// auto count = sf::Text(*font, std::format("{}", progressBar->GetProgress()), 15);
	// count.setPosition({ 800 - count.getLocalBounds().size.x, 0});
	//
	// auto percentage = sf::Text(*font, std::format("{:.0f}%", progressBar->GetPercentage() * 100), 15);
	// percentage.setPosition({ 800 - percentage.getLocalBounds().size.x, count.getPosition().y + count.getLocalBounds().size.y + 1 });
	//
    if (b_mContext->GFlags & GameFlags_ImGui) {
        {
            ImGui::Begin("Control for m_Shape"); 
            std::array size = { m_Shape.GetSize().X, m_Shape.GetSize().Y };
            ImGui::InputFloat2("Size (X, Y)", size.data());
            m_Shape.SetSize({ size[0], size[1] });
            std::array pos = { m_Shape.GetPosition().X, m_Shape.GetPosition().Y };
            ImGui::InputFloat2("Position (X, Y)", pos.data());
            m_Shape.SetPosition({ pos[0], pos[1] });
            ImGui::End();
        }

        {
            ImGui::Begin("Control for m_FunnyShape");
            std::array size = { m_FunnyShape.GetSize().X, m_FunnyShape.GetSize().Y };
            ImGui::InputFloat2("Size (X, Y)", size.data());
            m_FunnyShape.SetSize({ size[0], size[1] });
            std::array pos = { m_FunnyShape.GetPosition().X, m_FunnyShape.GetPosition().Y };
            ImGui::InputFloat2("Position (X, Y)", pos.data());
            m_FunnyShape.SetPosition({ pos[0], pos[1] });
            ImGui::End();
        }

        {
            ImGui::Begin("Control for m_Text");
            ImGui::Text("Position:");
            ImGui::SameLine();
            std::array size = { m_Text.GetPosition().X, m_Text.GetPosition().Y };
            ImGui::InputFloat2("(X, Y)", size.data());
            m_Text.SetPosition({ size[0], size[1] });
            ImGui::End();
        }
    }
    m_Text.Draw(mgr);
    m_Shape.Draw(mgr);
    m_FunnyShape.Draw(mgr);

    if (b_mContext->GFlags & GameFlags_ImGui)
        m_Shape.DrawWindowImGuiDrawData();

    if (m_HUD->GetRootElement<UI::Button>()->HasChild("musicProg")) {
        auto musicProgressBar = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::ProgressBar>("musicProg");
        
        musicProgressBar->Draw(mgr);
        // auto countMusic = sf::Text(*font, std::format("{}", musicProgressBar->GetProgress()), 15);
        // countMusic.setPosition({ 400 - countMusic.getGlobalBounds().size.x / 2, 300 });
        //
        // auto maxMusic = sf::Text(*font, std::format("{}", musicProgressBar->GetMaxValue()), 15);
        // maxMusic.setPosition({ 400 - maxMusic.getGlobalBounds().size.x / 2, countMusic.getPosition().y + maxMusic.getGlobalBounds().size.y - 1 });

        // window->draw(musicTitle);
        // window->draw(musicAlbum);
        // window->draw(musicTrackNum);
        // window->draw(musicYear);

        // window->draw(countMusic);
        // window->draw(maxMusic);
    }

	// window->draw(percentage);
	// window->draw(count);

    // b_mState->GetGraphicsMgr().GetRenderer()->SetGlobalShader(b_mState->GetAssetMgr().Get<Shader>("gradient").Get());
    b_mState->GetGraphicsMgr().ShowImGuiDebugWindow();
    b_mState->GetSettingsMgr().ShowImGuiDebugWindow();
    b_mState->GetAssetMgr().ShowImGuiDebugWindow();
    b_mState->GetAudioMgr().ShowImGuiDebugWindow();
    b_mState->GetUIMgr().ShowImGuiDebugWindow();
}

void TestView::OnEvent(Optional<Base::Event> event) {
    (void)event;
	m_HUD->OnEvent(event);
	//
    // if (auto data = event->GetIf<EventKeyPressed>()) {
    //     if (data->scancode == )
    //         SignalManager::GetInstance()->Send(std::make_shared<GameExitSignal>());
    // }
    if (event->Is<GameResizedEvent>()) {
        auto windowSize = b_mContext->WindowSize;
        auto btn = m_HUD->GetRootElement<UI::Button>()->GetChild<UI::Button>("gameBtn");
        btn->SetPosition({ static_cast<float>(windowSize.X / 2) - static_cast<float>(btn->GetSize().X / 2), 100 });
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
        musicProgressBar->UpdateProgress(std::floor(b_mState->GetAssetMgr().Get<AudioStream>("test-music", typeid(TestView)).Get()->GetTranscurredSeconds()));

        musicProgressBar->SetPosition({ 400 - (musicProgressBar->GetSize().X / 2), 590 });
        musicProgressBar->SetSize({ 300, 5 });
        musicProgressBar->Update(dt);
    }

	progressBar->SetPosition({ (b_mState->GetGraphicsMgr().GetWindow()->GetSize().X / 2) - (progressBar->GetSize().X / 2), 0});
	progressBar->SetSize({ 425, 5 });
	progressBar->Update(dt);
}

void TestView::OnExit() {
    Logger::AddInfo(typeid(TestView), "OnExit() called");
    // m_Shape.UninitGraphics(b_mState->GetGraphicsMgr());
    // m_FunnyShape.UninitGraphics(b_mState->GetGraphicsMgr());
    // m_Text.UninitGraphics(b_mState->GetGraphicsMgr());
}

void TestView::OnSuspend() {
    Logger::AddInfo(typeid(TestView), "OnSuspend() called");

    m_HUD->SetVisible(false);
}

void TestView::OnResume() {
    Logger::AddInfo(typeid(TestView), "OnResume() called");

    m_HUD->SetVisible(true);
}
