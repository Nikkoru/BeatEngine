#pragma once

#include <string>
#include <utility>
#include <map>
#include <memory>
#include <functional>

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/RectShape.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/UI/Alignment.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Optional.hpp"

class UIElement : public GraphicalElement {
protected:
    friend class UIManager;
	std::map<std::string, Texture> m_Textures;

	std::type_index m_ID = typeid(nullptr);

	Vector2f m_Size = { 0, 0 };
	Vector2f m_Position = { 0, 0 };
    RectShape m_LayoutRect;

    UIAlignmentV m_VAlignment = UIAlignmentV::Down;
    UIAlignmentH m_HAlignment = UIAlignmentH::Left;

	std::map<std::string, std::unique_ptr<UIElement>> m_Childs;

	bool m_Hidden = false;
	bool m_Active = false;

	std::function<void()> OnActive = nullptr;
	std::function<void()> OnDeactive = nullptr;

	std::function<void()> OnHide = nullptr;
	std::function<void()> OnShow = nullptr;

public:
	UIElement() = default;
	UIElement(std::type_index elementID) : m_ID(elementID) {}
	virtual ~UIElement() override;

	void SetOnActive(std::function<void()> func);
	void SetOnDeactive(std::function<void()> func);
	void SetOnHide(std::function<void()> func);
	void SetOnShow(std::function<void()> func);

	virtual void Show();
	virtual void Hide();

	void Update(float dt);
    virtual void OnUpdate(float dt) = 0;

	void SetSize(Vector2f size);
	void SetPosition(Vector2f position);
	void AddTexture(std::string name, const Texture& texture);
	void RemoveTexture(std::string name);

    void SetVAlignment(UIAlignmentV alignment);
    void SetHAlignment(UIAlignmentH alignment);

	Vector2f GetSize() const;
	Vector2f GetPosition() const;
    RectShape& GetLayoutRect() { return m_LayoutRect; }
	bool IsVisible() const;
public:
	bool HasChild() const;
	bool HasChild(const std::string& name) const;
    size_t ChildCount() const;
	void RemoveChild(const std::string& name);

	void OnEvent(Optional<Base::Event> event);
	virtual void EventHandler(Optional<Base::Event> event) { (void)event; }

	virtual void OnDraw(Renderer* const mgr, RenderState state ) = 0;
	void Draw(Renderer* const mgr, RenderState state = RenderState::Default) override;

    void UninitGraphics(Renderer* const mgr) override;
    virtual void OnUninitGraphics(Renderer* const mgr) { m_LayoutRect.UninitGraphics(mgr); }

    void DrawImGuiDrawData() override;

    virtual void SpecificImGuiDebug() {};

	template<typename TElement, typename... Args>
		requires(std::is_base_of_v<UIElement, TElement>)
	TElement* AddChild(const std::string& name, Args&&... constructorArgs) {
		for (auto& [childName, element] : m_Childs) {
			if (childName == name) {
				Logger::AddError("", "Element \"{}\" already exists in container", name);
				return nullptr;
			}
		}
		
		m_Childs.emplace(name, std::make_unique<TElement>(std::forward<Args>(constructorArgs)...));

		return static_cast<TElement*>(m_Childs.at(name).get());
	}
	template<typename TElement>
		requires(std::is_base_of_v<UIElement, TElement>)
	TElement* GetChild(const std::string& name) {
        if (!m_Childs.contains(name)) {
            std::string msg = "Element \"" + name + "\" doesn't exists in container";
            Logger::AddCritical("", msg);
            THROW_RUNTIME_ERROR(msg);
        }

        return static_cast<TElement*>(m_Childs.at(name).get());
    }
};
