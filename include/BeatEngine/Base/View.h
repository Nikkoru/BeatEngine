#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <typeindex>

#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/AudioManager.h"

class ViewManager;

namespace Base {
	class View {
	public:
		std::type_index b_ID;
	protected:
		bool b_mSuspended = false;
		AssetManager* b_mAssetMgr = nullptr;
		SettingsManager* b_mSettingsMgr = nullptr;
		AudioManager* b_mAudioMgr = nullptr;
	private:
		friend class ViewManager;
	public:
		bool operator==(const View& other) const;
	public:
		/// <summary>
		/// Empty constructor. 
		/// </summary>
		View() : b_ID(typeid(nullptr)) {}
		/// <summary>
		/// Creates a view with a derivated view type_index an a pointer to the managers
		/// </summary>
		/// <param name="id">the derivated view type_index</param>
		/// <param name="assetMgr">the AssetManager pointer</param>
		/// <param name="settingsMgr">the SettingsManager pointer</param>
		/// <param name="audioMgr">the AudioManager pointer</param>
		View(std::type_index id, AssetManager* assetMgr = nullptr, SettingsManager* settingsMgr = nullptr, AudioManager* audioMgr = nullptr) 
			: b_ID(id), b_mAssetMgr(assetMgr), b_mSettingsMgr(settingsMgr), b_mAudioMgr(audioMgr) {}

		virtual ~View() = default;
	public:
		/// <summary>
		/// Notifies the view to draw to the given window.
		/// </summary>
		/// <param name="window">the SFML window to draw</param>
		virtual void OnDraw(sf::RenderWindow* window) = 0;
		/// <summary>
		/// Notifies the view when SFML event is trigged
		/// </summary>
		/// <param name="event">the SFML event</param>
		virtual void OnSFMLEvent(const std::optional<sf::Event> event) = 0;
		/// <summary>
		/// Notifies the view to update.
		/// </summary>
		virtual void OnUpdate(float dt) = 0;
		/// <summary>
		/// Notifies the view to suspend.
		/// </summary>
		virtual void OnSuspend();
		/// <summary>
		/// Notifies the view to resume.
		/// </summary>
		virtual void OnResume();
		virtual void OnExit() = 0;
		/// <summary>
		/// Indicates when the view is suspended.
		/// </summary>
		/// <returns>the suspended status</returns>
		bool IsSuspended() const;
	private:
		/// <summary>
		/// Sets the AssetManager of the manager
		/// </summary>
		/// <param name="assetMgr">the AssetManager</param>
		void SetAssetManager(AssetManager* assetMgr);
		void SetSettingsManager(SettingsManager* settingsMgr);
	};
}