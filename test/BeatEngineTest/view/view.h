#pragma once

#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Camera/Camera.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/RectShape.hpp"
#include "BeatEngine/Graphics/TextElement.hpp"
#include "BeatEngine/Manager/GraphicsManager.h"
#include <BeatEngine/Base/View.h>
#include <BeatEngine/Asset/Font.h>
#include <BeatEngine/UI/Elements/Button.h>
#include <BeatEngine/UI/Elements/ProgressBar.h>
#include <memory>
#include <string>

class TestView : public Base::View {
private:
    Camera m_Camera{};

    std::shared_ptr<UILayer> m_HUD = nullptr;

	std::wstring m_MusicTitleText;
	std::wstring m_MusicAlbumText;
	std::wstring m_MusicTrackNumText;
	std::wstring m_MusicYearText;

	float m_Timer = 0;
    Base::AssetHandle<Font> m_Font{};

	bool m_buttonActive = false;

    RectShape m_Shape{};
    RectShape m_FunnyShape{};
    TextElement m_Text{};

	float progress = 0;
public:
	TestView(GameContext* context, GameState* state);
	~TestView() override = default;
public:
    void Init() override;
	void OnDraw(GraphicsManager& mgr) override;
	void OnEvent(Optional<Base::Event> event) override;
	void OnUpdate(float dt) override;
	void OnExit() override;

    void OnSuspend() override;
    void OnResume() override;
};
