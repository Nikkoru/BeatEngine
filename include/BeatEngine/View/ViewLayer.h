#pragma once

#include "BeatEngine/AppState.hpp"
#include "BeatEngine/Camera/Camera.h"
#include "BeatEngine/Camera/CameraMode.h"
#include "BeatEngine/Camera/ShakeParams.h"
#include "BeatEngine/AppContext.hpp"
#include "BeatEngine/Manager/AudioManager.h"
#include "BeatEngine/Manager/SystemManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Manager/SettingsManager.h"

#include <optional>
#include <memory>
#include <typeindex>

namespace Base {
	class View;
}

class Application;
class ViewLayerStack;
class GraphicsManager;
class ViewLayer {
private:
	friend class ViewLayerStack;
private:
	std::shared_ptr<Base::View> m_OwnerView = nullptr;
	std::type_index m_ID = typeid(nullptr);
protected:
    AppContext* m_Context{ nullptr };
    AppState* m_State{ nullptr };
private:
	Vector2f m_Size = { 0, 0 };
	unsigned int m_LayerIndex = 0;
	
    Camera m_Camera;

    // sf::View m_MainView; 
private:
	void SetLayerIndex(unsigned int index) { m_LayerIndex = index; }
public:
	ViewLayer(std::type_index id,
              AppContext* context = nullptr,
              AppState* state = nullptr)
		: m_ID(id), m_Context(context), m_State(state) {}

	virtual ~ViewLayer() = default;

    virtual void Init() = 0;

	virtual void OnUpdate(float dt) = 0;
	virtual void OnAttach() {} 
	virtual void OnDetach() {}
	virtual void OnEvent(Optional<Base::Event> event) = 0;
    virtual void OnDraw(GraphicsManager& mgr, RenderState state) = 0;

	inline Vector2f GetSize() const { return m_Size; }
	inline float GetCameraZoom() const { return m_Camera.GetZoom(); };
	inline std::shared_ptr<Base::View> GetOwner() const { return m_OwnerView; };

	void SetCamera(Camera& camera);
	void SetCameraMode(CameraMode mode);
	void SetCameraPosition(Vector2f pos);
	void SetCameraRotation(float rotation);
	void SetCameraZoom(float zoom);
	void ShakeCamera(ShakeParams params);
	Vector2f GetScreenToWorld(Vector2f pos) const;
	Vector2f GetWorldToScreen(Vector2f pos) const;
	void StartCamera();
	void StopCamera();
    
    void SetAppContext(AppContext* context) { m_Context = context; }
    void SetAppState(AppState* state) { m_State = state; }
private:
	friend class Application;
};
