#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include "BeatEngine/Camera/Camera.h"
#include "BeatEngine/Camera/CameraMode.h"
#include "BeatEngine/Camera/ShakeParams.h"
#include "BeatEngine/GameContext.h"
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
    SettingsManager* m_SettingsMgr = nullptr;
    AudioManager* m_AudioMgr = nullptr;
    SystemManager* m_SystemMgr = nullptr;
protected:
    GameContext* m_Context;
private:
	sf::Vector2f m_Size = { 0, 0 };
	unsigned int m_LayerIndex = 0;
	
    Camera m_Camera;

    sf::View m_MainView; 
private:
	void SetLayerIndex(unsigned int index) { m_LayerIndex = index; }
public:
	ViewLayer(std::type_index id,
              GameContext* context = nullptr,
              UIManager* uiMgr = nullptr, 
              AssetManager* assetMgr = nullptr, 
              SettingsManager* settingsMgr = nullptr, 
              AudioManager* audioMgr = nullptr, 
              SystemManager* systemMgr = nullptr)
		: m_ID(id), m_Context(context), m_UIMgr(uiMgr), m_AssetMgr(assetMgr), m_SettingsMgr(settingsMgr), m_AudioMgr(audioMgr), m_SystemMgr(systemMgr) {}

	virtual ~ViewLayer() = default;

	virtual void OnUpdate(float dt) = 0;
	virtual void OnAttach() {} 
	virtual void OnDetach() {}
	virtual void OnSFMLEvent(std::optional<sf::Event> event) = 0;

	inline sf::Vector2f GetSize() const { return m_Size; }
	inline float GetCameraZoom() const { return m_Camera.GetZoom(); };
	inline std::shared_ptr<Base::View> GetOwner() const { return m_OwnerView; };

	void SetCamera(Camera& camera);
	void SetCameraMode(CameraMode mode);
	void SetCameraPosition(sf::Vector2f pos);
	void SetCameraRotation(float rotation);
	void SetCameraZoom(float zoom);
	void ShakeCamera(ShakeParams params);
	sf::Vector2f GetScreenToWorld(sf::Vector2f pos) const;
	sf::Vector2f GetWorldToScreen(sf::Vector2f pos) const;
	void StartCamera();
	void StopCamera();
    
    void SetGameContext(GameContext* context) { m_Context = context; }
	void SetUIManager(UIManager* mgr) { m_UIMgr = mgr; }
    void SetAssetManager(AssetManager* mgr) { m_AssetMgr = mgr; }
    void SetSettingsManager(SettingsManager* mgr) { m_SettingsMgr = mgr; }
    void SetAudioManager(AudioManager* mgr) { m_AudioMgr = mgr; }
    void SetSystemManager(SystemManager* mgr) { m_SystemMgr = mgr; }
private:
	friend class Game;
};
