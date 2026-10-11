#include "BeatEngine/UI/UILayer.h"

template <typename TUI, typename... Args> 
    requires(std::is_base_of_v<UIElement, TUI>)
TUI* UILayer::SetRootElement(Args&&... constructorArgs) {
	m_Root = std::make_unique<TUI>(std::forward<Args>(constructorArgs)...);
	
	return GetRootElement<TUI>();
}

template<typename TUI>
	requires(std::is_base_of_v<UIElement, TUI>)
TUI* UILayer::GetRootElement() {
	return static_cast<TUI*>(m_Root.get());
}
