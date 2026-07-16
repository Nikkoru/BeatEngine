#include "BeatEngine/UI/UIElement.h"

#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/UI/Alignment.h"
#include "BeatEngine/Util/UIHelper.h"
#include <memory>

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
    
    for (const auto& [name, element] : m_Childs)
        element->Show();
}

void UIElement::Hide() {
	m_Hidden = true;

	if (OnHide)
		OnHide();

    for (const auto& [name, element] : m_Childs)
        element->Hide();
}

void UIElement::Update(float dt) {
    OnUpdate(dt);

    if (!m_Childs.empty())
        for (auto& [childName, element] : m_Childs)
            element->Update(dt);
}

void UIElement::SetSize(Vector2f size) {
	this->m_Size = size;
	this->m_LayoutRect.SetSize(size);
}

void UIElement::SetPosition(Vector2f position) {
	this->m_Position = position;
    this->m_LayoutRect.SetPosition(position);
}

void UIElement::AddTexture(std::string name, const Texture& texture) {
	if (!m_Textures.contains(name))
		m_Textures.try_emplace(name, texture);
}

void UIElement::RemoveTexture(std::string name) {
	if (!m_Textures.contains("name"))
		m_Textures.erase(name);
}

void UIElement::SetVAlignment(UIAlignmentV alignment) {
    this->m_VAlignment = alignment;
}

void UIElement::SetHAlignment(UIAlignmentH alignment) {
    this->m_HAlignment = alignment;
}

Vector2f UIElement::GetSize() const {
	return m_Size;
}

Vector2f UIElement::GetPosition() const {
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
    return m_Childs.size();
}

void UIElement::RemoveChild(const std::string& name) {
	for (auto& [childName, element] : m_Childs) {
		if (childName == name) {
			auto it = m_Childs.find(childName);
            SignalManager::GetInstance()->Send(std::make_shared<GameUninitGraphicsSignal>(*it->second));
			m_Childs.erase(it);
		}
	}
}

void UIElement::OnEvent(Optional<Base::Event> event) {
	EventHandler(event);

	if (!m_Childs.empty())
		for (const auto& [childName, element] : m_Childs)
			element->EventHandler(event);
}

void UIElement::Draw(GraphicsManager& mgr) {
    if (!m_Hidden)
	    OnDraw(mgr);

	if (m_Childs.empty()) return;

    for (const auto& [childName, element] : m_Childs)
        if (!element->m_Hidden)
            element->OnDraw(mgr);
}

void UIElement::UninitGraphics(GraphicsManager& mgr) {
    OnUninitGraphics(mgr);

    if (m_Childs.empty()) return;

    for (const auto& [childName, element] : m_Childs) {
        element->UninitGraphics(mgr);
    }
}

void UIElement::DrawImGuiDrawData() {
    m_LayoutRect.DrawImGuiDrawData();

    // for (const auto& [childName, element] : m_Childs) {
    //     element->DrawImGuiDrawData();
    // }
}
