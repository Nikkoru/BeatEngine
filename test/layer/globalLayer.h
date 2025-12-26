#pragma once

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include <BeatEngine/View/ViewLayer.h>
#include <BeatEngine/UI/UILayer.h>

#include <memory>
#include <string>

class GlobalTestLayerUI : public ViewLayer {
private:
	std::shared_ptr<UILayer> m_HUD = nullptr;
	std::string m_FPSText;
    std::string m_DeltaText;
    std::shared_ptr<Font> m_Font = nullptr;
public:
	GlobalTestLayerUI();
	GlobalTestLayerUI(UIManager* uiMgr, AssetManager* assetMgr, SettingsManager* settingsMgr);
	~GlobalTestLayerUI() override = default;
private:

public:
	void OnUpdate(float dt) override;
	void OnAttach() override;
	void OnDetach() override;
	void OnSFMLEvent(std::optional<sf::Event> event) override;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
