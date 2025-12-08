#include "BeatEngine/UI/UILayer.h"

template<typename TUI>
	requires(std::is_base_of_v<UIElement, TUI>)
std::shared_ptr<TUI> UILayer::SetRootElement() {
	m_Root = std::make_shared<TUI>();
	
	return std::static_pointer_cast<TUI>(m_Root);
}

template<typename TUI>
	requires(std::is_base_of_v<UIElement, TUI>)
std::shared_ptr<TUI> UILayer::GetRootElement() {
	return std::static_pointer_cast<TUI>(m_Root);
}