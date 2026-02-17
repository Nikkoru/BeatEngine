#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Exception.h"
#include <format>

template <typename TAsset>
    requires(std::is_base_of_v<Base::Asset, TAsset>)
Base::AssetHandle<TAsset> AssetManager::Get(const std::string assetName, const std::type_index viewID) {
    if (viewID == typeid(nullptr)) {
        if (m_GlobalAssets.contains(assetName))
            return Base::AssetHandle<TAsset>::Cast(m_GlobalAssets.at(assetName).Handle);
        else {
            std::string msg = "Asset not found: \"" + assetName + "\"";
            Logger::AddCritical(typeid(AssetManager), msg);
            THROW_RUNTIME_ERROR(msg);
        }
    }
    else {
        if (m_ViewAssets.contains(viewID)) {
            if (m_ViewAssets.at(viewID).contains(assetName))
                return Base::AssetHandle<TAsset>::Cast(m_ViewAssets.at(viewID).at(assetName).Handle);
            else {
                std::string msg = "Asset not found: \"" + assetName + "\"";
                Logger::AddCritical(typeid(AssetManager), msg);
                THROW_RUNTIME_ERROR(msg);
            }
        }
        else {
            std::string msg = std::format("View {} dosen't have assets", viewID.name());
            Logger::AddCritical(typeid(AssetManager), msg);
            THROW_RUNTIME_ERROR(msg);
        }
    }
}
