#include "BeatEngine/UI/UILayer.h"

template<typename TUI>
	requires(std::is_base_of_v<UIElement, TUI>)
TUI* UILayer::SetRootElement() {
	m_Root = std::make_unique<TUI>();
	
	return GetRootElement<TUI>();
}

template<typename TUI>
	requires(std::is_base_of_v<UIElement, TUI>)
TUI* UILayer::GetRootElement() {
	return static_cast<TUI*>(m_Root.get());
}
