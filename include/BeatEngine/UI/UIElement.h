#pragma once

#include <string>
#include <utility>
#include <map>
#include <memory>
#include <functional>
#include <SFML/Graphics.hpp>

#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/UI/Alignment.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Logger.h"

/// <summary>
/// Semi-abstract base class for UI Elements compatible with SFML.
/// As is compatible with SFML, it can draw its components using the normal <code>window.draw(UIElement)</code> method which each derivated class needs to implement.
/// </summary>
class UIElement : public sf::Drawable {
protected:
	std::map<std::string, Texture> m_Textures;

	std::type_index m_ID = typeid(nullptr);

	sf::Vector2f m_Size = { 0, 0 };
	sf::Vector2f m_Position = { 0, 0 };;
	sf::RectangleShape m_LayoutRect = sf::RectangleShape(m_Size);

    UIAlignmentV m_VAlignment = UIAlignmentV::Down;
    UIAlignmentH m_HAlignment = UIAlignmentH::Left;

	std::map<std::string, std::shared_ptr<UIElement>> m_Childs;

	bool m_Hidden = false;
	bool m_Active = false;

	std::function<void()> OnActive = nullptr;
	std::function<void()> OnDeactive = nullptr;

	std::function<void()> OnHide = nullptr;
	std::function<void()> OnShow = nullptr;

public:
	UIElement() = default;
	UIElement(std::type_index elementID) : m_ID(elementID) {}
	virtual ~UIElement();

	void SetOnActive(std::function<void()> func);
	void SetOnDeactive(std::function<void()> func);
	void SetOnHide(std::function<void()> func);
	void SetOnShow(std::function<void()> func);

	virtual void Show();
	virtual void Hide();

	virtual void Update(float dt) = 0;

	void SetSize(sf::Vector2f size);
	void SetPosition(sf::Vector2f position);
	void AddTexture(std::string name, const Texture& texture);
	void RemoveTexture(std::string name);

    void SetVAlignment(UIAlignmentV alignment);
    void SetHAlignment(UIAlignmentH alignment);

	sf::Vector2f GetSize() const;
	sf::Vector2f GetPosition() const;
	bool IsVisible() const;
public:
	bool HasChild() const;
	bool HasChild(const std::string& name) const;
    size_t ChildCount() const;
	void RemoveChild(const std::string& name);

	void OnSFMLEvent(std::optional<sf::Event> event);
	virtual void EventHandler(std::optional<sf::Event> event) {}

	virtual void OnDraw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	template<typename TElement>
		requires(std::is_base_of_v<UIElement, TElement>)
	inline std::shared_ptr<TElement> AddChild(const std::string& name) {
		for (auto& [childName, element] : m_Childs) {
			if (childName == name) {
				Logger::GetInstance()->AddError("Element" + name + " already exists in container", m_ID);
				return nullptr;
			}
		}

		auto element = std::make_shared<TElement>();
		m_Childs.emplace(name, element);

		return element;
	}
	template<typename TElement>
		requires(std::is_base_of_v<UIElement, TElement>)
	inline std::shared_ptr<TElement> GetChild(const std::string& name) {
		for (auto& [childName, element] : m_Childs) {
			if (childName == name) {
				return std::static_pointer_cast<TElement>(element);
			}
		}

		std::string msg = "Element \"" + name + "\" doesn't exists in container";
		Logger::GetInstance()->AddCritical(msg, m_ID);
		THROW_RUNTIME_ERROR(msg);
	}
};
