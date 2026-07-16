#include "BeatEngine/UI/UILayer.h"
#include "BeatEngine/Manager/GraphicsManager.h"

UILayer::UILayer(Vector2f size, Vector2f position) {
    // TODO: why is this constructor here even
    (void)size;
    (void)position;
}

void UILayer::SetLayerBackPanel() {
	m_BackPanel = std::make_shared<UIPanel>();
}

void UILayer::SetLayerBackPanel(std::shared_ptr<UIPanel> backPanel) {
	m_BackPanel = backPanel;
}

void UILayer::OnEvent(Optional<Base::Event> event) {
	if (m_Root && !m_Hidden)
		m_Root->OnEvent(event);
}

void UILayer::Update(float dt) {
	if (m_Root && !m_Hidden)
		m_Root->Update(dt);
}

void UILayer::Draw(GraphicsManager& mgr) {
    if (m_BackPanel)
        m_BackPanel->Draw(mgr);

	if (m_Root && !m_Hidden)
		m_Root->Draw(mgr);
}

void UILayer::UninitGraphics(GraphicsManager& mgr) {
    if (m_BackPanel)
        m_BackPanel->UninitGraphics(mgr);

    if (m_Root)
        m_Root->UninitGraphics(mgr);
}

void UILayer::SetVisible(bool visible) {
    this->m_Hidden = !visible;
}
