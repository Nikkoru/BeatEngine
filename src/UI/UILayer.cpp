#include "BeatEngine/UI/UILayer.h"

#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/UI/UIElement.h"
#include "BeatEngine/UI/Elements/UIPanel.h"

UILayer::UILayer(Vector2f size, Vector2f position) {
    // TODO: why is this constructor here even
    (void)size;
    (void)position;
}

UILayer::~UILayer() = default;

void UILayer::SetLayerBackPanel() {
	m_BackPanel = std::make_unique<UIPanel>();
}

void UILayer::OnEvent(Optional<Base::Event> event) {
	if (m_Root && !m_Hidden)
		m_Root->OnEvent(event);
}

void UILayer::Update(float dt) {
	if (m_Root && !m_Hidden)
		m_Root->Update(dt);
}

void UILayer::Draw(Renderer* const mgr, RenderState state) {
    if (m_BackPanel) {
        m_BackPanel->Draw(mgr, state);
    }

	if (m_Root && !m_Hidden) {
		m_Root->Draw(mgr, state);
    }
}

void UILayer::UninitGraphics(Renderer* const mgr) {
    if (m_BackPanel)
        m_BackPanel->UninitGraphics(mgr);

    if (m_Root)
        m_Root->UninitGraphics(mgr);
}

void UILayer::SetVisible(bool visible) {
    this->m_Hidden = !visible;
}
