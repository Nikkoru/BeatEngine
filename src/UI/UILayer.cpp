#include "BeatEngine/UI/UILayer.h"

UILayer::UILayer(Vector2f size, Vector2f position) {
}

void UILayer::SetLayerBackPanel() {
	m_BackPanel = std::make_shared<UIPanel>();
}

void UILayer::SetLayerBackPanel(std::shared_ptr<UIPanel> backPanel) {
	m_BackPanel = backPanel;
}

void UILayer::OnEvent(std::optional<Base::Event> event) {
	if (m_Root && !m_Hidden)
		m_Root->OnEvent(event);
}

void UILayer::Update(float dt) {
	if (m_Root && !m_Hidden)
		m_Root->Update(dt);
}

// void UILayer::draw(/*sf::RenderTarget& target, sf::RenderStates states*/) const {
// 	//m_BackPanel->Draw(window);
// 	// if (m_Root && !m_Hidden)
// 	// 	target.draw(*m_Root);
// }

void UILayer::SetVisible(bool visible) {
    this->m_Hidden = !visible;
}
