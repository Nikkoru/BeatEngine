#pragma once

#include <BeatEngine/Base/View.h>
#include <BeatEngine/Asset/Font.h>
#include <BeatEngine/UI/Elements/Button.h>
#include <BeatEngine/UI/Elements/ProgressBar.h>

class TestView : public Base::View {
private:
	UI::Button m_Button;
	UI::Button m_Play;
	UI::Button m_Pause;
	UI::Button m_Stop;
	UI::ProgressBar m_ProgressBar;

	std::string m_FPSDeltaTimeText;
	float m_Timer = 0;
	std::shared_ptr<Font> m_Font = nullptr;

	bool m_buttonActive = false;

	float progress = 0;
public:
	TestView(AssetManager* assetMgr, AudioManager* audioMgr, SettingsManager* settingsMgr);
	~TestView() override = default;
public:
	void OnDraw(sf::RenderWindow* window) override;
	void OnSFMLEvent(std::optional<sf::Event> event) override;
	void OnUpdate(float dt) override;
	void OnExit() override;
};