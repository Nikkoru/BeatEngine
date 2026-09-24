#pragma once

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/AppContext.hpp"
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
	GlobalTestLayerUI(AppContext* context, AppState* state);
	~GlobalTestLayerUI() override = default;
private:

public:
    void Init() override;

	void OnUpdate(float dt) override;
	void OnAttach() override;
	void OnDetach() override;
	void OnEvent(Optional<Base::Event> event) override;
	void OnDraw(Renderer* const mgr, RenderState state) override;

    void ToggleImGuiDrawing();

    void UpdatePositions();

    void DrawImGuiDebug() const;
};
