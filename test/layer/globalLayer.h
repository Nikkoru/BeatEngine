#pragma once

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/View/ViewLayer.h"
#include "BeatEngine/UI/UILayer.h"

#include <memory>
#include <string>

class GlobalTestLayerUI : public ViewLayer {
private:
	std::shared_ptr<UILayer> m_HUD = nullptr;
	std::string m_FPSText;
    std::string m_DeltaText;
    std::shared_ptr<Font> m_Font = nullptr;
    bool m_DrawDebug = false;
public:
	GlobalTestLayerUI();
	GlobalTestLayerUI(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state);
	~GlobalTestLayerUI() override = default;
private:

public:
	void OnUpdate(float dt) override;
	void OnAttach() override;
	void OnDetach() override;
	void OnEvent(std::optional<Base::Event> event) override;

	void draw(/*sf::RenderTarget& target, sf::RenderStates states*/) const /*override*/;

    void ToggleImGuiDrawing();

    void UpdatePositions();

    void DrawImGuiDebug() const;
};
