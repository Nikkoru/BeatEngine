#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Manager/AssetManager.h"

#include <optional>
#include <memory>
#include <typeindex>

namespace Base {
	class View;
}

class Game;
class ViewLayerStack;
class ViewLayer : public sf::Drawable {
private:
	friend class ViewLayerStack;
private:
	std::shared_ptr<Base::View> m_OwnerView = nullptr;
	std::type_index m_ID = typeid(nullptr);
protected:
	UIManager* m_UIMgr = nullptr;
	AssetManager* m_AssetMgr = nullptr;
private:
	sf::Vector2f m_Size = { 0, 0 };
	unsigned int m_LayerIndex = 0;

	void* m_Camera = nullptr;
private:
	void SetLayerIndex(unsigned int index) { m_LayerIndex = index; }
public:
	ViewLayer(std::type_index id, UIManager* uiMgr = nullptr, AssetManager* assetMgr = nullptr)
		: m_ID(id), m_UIMgr(uiMgr), m_AssetMgr(assetMgr) {}

	virtual ~ViewLayer() = default;

	virtual void OnUpdate(float dt) = 0;
	virtual void OnAttach() = 0;
	virtual void OnDetach() = 0;
	virtual void OnSFMLEvent(std::optional<sf::Event> event) = 0;

	inline sf::Vector2f GetSize() const { return m_Size; }
	inline float GetCameraZoom()  const	{ return .0f; };
	inline std::shared_ptr<Base::View> GetOwner() const { return m_OwnerView; };

	void SetCamera(void* noCameraYet) {};
	void SetCameraMode(void* noCameraModeYet) {};
	void SetCameraOffset(sf::Vector2f offset) {};
	void SetCameraRotation(float rotation) {};
	void SetCameraZoom(float zoom) {};
	void ShakeCamera(void* noShakeParamsYet) {};
	sf::Vector2f GetScreenToWorld(sf::Vector2f pos) const { return { 0, 0 }; };
	sf::Vector2f GetWorldToScreen(sf::Vector2f pos) const { return { 0, 0 }; };
	void StartCamera() {};
	void StopCamera() {};
private:
	friend class Game;
	void SetUIManager(UIManager* mgr) { m_UIMgr = mgr; }
};