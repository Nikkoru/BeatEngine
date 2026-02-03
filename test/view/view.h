#pragma once

#include "BeatEngine/GameContext.h"
#include <BeatEngine/Base/View.h>
#include <BeatEngine/Asset/Font.h>
#include <BeatEngine/UI/Elements/Button.h>
#include <BeatEngine/UI/Elements/ProgressBar.h>
#include <string>

class TestView : public Base::View {
private:
    std::shared_ptr<UILayer> m_HUD = nullptr;

	std::wstring m_MusicTitleText;
	std::wstring m_MusicAlbumText;
	std::wstring m_MusicTrackNumText;
	std::wstring m_MusicYearText;

	float m_Timer = 0;
	std::shared_ptr<Font> m_Font = nullptr;

	bool m_buttonActive = false;

	float progress = 0;
public:
	TestView(GameContext* context, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr);
	~TestView() override = default;
public:
	void OnDraw(sf::RenderWindow* window) override;
	void OnSFMLEvent(std::optional<sf::Event> event) override;
	void OnUpdate(float dt) override;
	void OnExit() override;

    void OnSuspend() override;
    void OnResume() override;
};
