#include "BeatEngine/UI/UIElement.h"

#include "BeatEngine/Util/UIHelper.h"

UIElement::~UIElement() {
}

void UIElement::SetOnActive(std::function<void()> func) {
	this->OnActive = func;
}

void UIElement::SetOnDeactive(std::function<void()> func) {
	this->OnDeactive = func;
}

void UIElement::SetOnHide(std::function<void()> func) {
	this->OnHide = func;
}

void UIElement::SetOnShow(std::function<void()> func) {
	this->OnShow = func;
}

void UIElement::Show() {
	m_Hidden = false;

	if (OnShow)
		OnShow();
}

void UIElement::Hide() {
	m_Hidden = true;

	if (OnHide)
		OnHide();
}

void UIElement::Update(float dt) {
    if (!m_Childs.empty())
        for (auto& [childName, element] : m_Childs)
            element->Update(dt);
}

void UIElement::SetSize(sf::Vector2f size) {
	this->m_Size = size;
	this->m_LayoutRect.setSize(size);
}

void UIElement::SetPosition(sf::Vector2f position) {
	this->m_Position = position;
}

void UIElement::AddTexture(std::string name, const Texture& texture) {
	if (!m_Textures.contains(name))
		m_Textures.try_emplace(name, texture);
}

void UIElement::RemoveTexture(std::string name) {
	if (!m_Textures.contains("name"))
		m_Textures.erase(name);
}

sf::Vector2f UIElement::GetSize() const {
	return m_Size;
}

sf::Vector2f UIElement::GetPosition() const {
	return m_Position;
}

bool UIElement::IsVisible() const {
	return !m_Hidden;
}

bool UIElement::HasChild() const {
	return m_Childs.empty();
}

bool UIElement::HasChild(const std::string& name) const {
	for (auto& [childName, element] : m_Childs) {
		if (childName == name)
			return true;
	}
	return false;
}

size_t UIElement::ChildCount() const {
    return m_Childs.count();
}

void UIElement::RemoveChild(const std::string& name) {
	for (auto& [childName, element] : m_Childs) {
		if (childName == name) {
			auto it = m_Childs.find(childName);
			m_Childs.erase(it);
		}
	}
}

void UIElement::OnSFMLEvent(std::optional<sf::Event> event) {
	EventHandler(event);

	if (!m_Childs.empty())
		for (const auto& [childName, element] : m_Childs)
			element->EventHandler(event);
}

void UIElement::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	OnDraw(target, states);

	if (!m_Childs.empty())
		for (const auto& [childName, element] : m_Childs)
			element->OnDraw(target, states);
}
