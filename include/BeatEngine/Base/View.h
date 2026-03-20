#pragma once

#include <memory>
#include <optional>
#include <typeindex>

#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"

#include "BeatEngine/View/ViewLayerStack.h"

class ViewManager;

namespace Base {
	class View {
	public:
		std::type_index b_ID;
	protected:
		bool b_mSuspended = false;
    protected:
        std::shared_ptr<GameContext> b_mContext{ nullptr };
        std::shared_ptr<GameState> b_mState{ nullptr };
	protected:
		ViewLayerStack b_mLayerStack;
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
		/// <param name="uiMgr">the UIManager pointer</param>
		View(std::type_index id, std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state = nullptr) 
			: b_ID(id), b_mContext(context), b_mState(state) {}

		virtual ~View() = default;
	public: // Events
		/// <summary>
		/// Notifies the view to draw to the given window.
		/// </summary>
		/// <param name="window">the SFML window to draw</param>
		virtual void OnDraw(GraphicsManager* window) = 0;
		/// <summary>
		/// Notifies the view when SFML event is trigged
		/// </summary>
		/// <param name="event">the SFML event</param>
		virtual void OnEvent(const std::optional<Base::Event> event) = 0;
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
	};
}
