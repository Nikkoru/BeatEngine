#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <typeindex>


namespace Base {
	class View {
	public:
		bool operator==(const View& other) const;
	public:
		View() : ID(typeid(nullptr)) {}
		View(std::type_index id) : ID(id) {}
		virtual ~View() = default;
	public:
		virtual void OnDraw(sf::RenderWindow* window) = 0;
		virtual void OnSFMLEvent(const std::optional<sf::Event> event) = 0;
		virtual void OnUpdate() = 0;
		virtual void OnSuspend();
		virtual void OnResume();
		bool IsSuspended() const;
	public:
		std::type_index ID;
	protected:
		bool m_Suspended = false;
	};
}