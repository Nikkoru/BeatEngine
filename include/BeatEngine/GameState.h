#pragma once

#include <memory>

class ViewManager;
class SystemManager;
class AssetManager;
class SettingsManager;
class AudioManager;
class UIManager;
class GraphicsManager;
struct GameState {
public:
    std::shared_ptr<ViewManager> ViewMgr{ nullptr };
	std::shared_ptr<SystemManager> SystemMgr{ nullptr };
	std::shared_ptr<AssetManager> AssetMgr{ nullptr };
	std::shared_ptr<SettingsManager> SettingsMgr{ nullptr };
	std::shared_ptr<AudioManager> AudioMgr{ nullptr };
	std::shared_ptr<UIManager> UIMgr{ nullptr };
    std::shared_ptr<GraphicsManager> GraphicsMgr{ nullptr };
};
