#include "GameLayer.h"
#include "BeatEngine/View/ViewLayer.h"



GameLayer::GameLayer(GameContext* context, 
              UIManager* uiMgr, 
              AssetManager* assetMgr,
              SettingsManager* settingsMgr,
              AudioManager* audioMgr,
              SystemManager* systemMgr) 
    : ViewLayer(typeid(GameLayer),
                context,
                uiMgr,
                assetMgr,
                settingsMgr,
                audioMgr,
                systemMgr) 
{}
