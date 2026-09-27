#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Exception.h"
#include <format>

template <typename TAsset>
    requires(std::is_base_of_v<Base::Asset, TAsset>)
void AssetManager::SetLoadCallback(AssetLoadCallback callback) {
    if (m_LoadCallbacks.contains(typeid(TAsset))) {
        Logger::AddWarning(typeid(AssetManager), "Asset type \"{}\" already has a load callback", typeid(TAsset).name());
        return;
    } 

    m_LoadCallbacks[typeid(TAsset)] = callback;
    Logger::AddDebug(typeid(AssetManager), "Added callback for asset type: \"{}\"", typeid(TAsset).name());
}

template <typename TAsset>
    requires(std::is_base_of_v<Base::Asset, TAsset>)
void AssetManager::SetUnloadCallback(AssetUnloadCallback callback) {
    if (m_UnloadCallbacks.contains(typeid(TAsset))) {
        Logger::AddWarning(typeid(AssetManager), "Asset type \"{}\" already has a unload callback", typeid(TAsset).name());
        return;
    }

    m_UnloadCallbacks[typeid(TAsset)] = callback;
    Logger::AddDebug(typeid(AssetManager), "Added unload callback for asset type: \"{}\"", typeid(TAsset).name());
}

template <typename TAsset>
    requires(std::is_base_of_v<Base::Asset, TAsset>)
Base::AssetHandle<TAsset> AssetManager::Load(const fs::path& path, std::type_index viewID) {
    std::string assetName = typeid(TAsset).name();
    if (!m_LoadCallbacks.contains(typeid(TAsset))) {
        auto msg = std::format("Load failed for file in \"{}\": Theres no callback available for this type -> {}", path.string(), assetName);
        Logger::AddCritical(typeid(AssetManager), msg);
        THROW_RUNTIME_ERROR(msg);
    }
    auto handle = m_LoadCallbacks.at(typeid(TAsset))(path);
    return Base::AssetHandle<TAsset>::Cast(handle);
}
template <typename TAsset>
    requires(std::is_base_of_v<Base::Asset, TAsset>)
Base::AssetHandle<TAsset> AssetManager::Get(const String& assetName, const std::type_index viewID) {
    if (viewID == typeid(nullptr)) {
        if (m_GlobalAssets.contains(assetName))
            return Base::AssetHandle<TAsset>::Cast(m_GlobalAssets.at(assetName).Handle);
        else {
            std::string msg = "Asset not found: \"" + assetName.ToString(true) + "\"";
            Logger::AddCritical(typeid(AssetManager), msg);
            THROW_RUNTIME_ERROR(msg);
        }
    }
    else {
        if (m_ViewAssets.contains(viewID)) {
            if (m_ViewAssets.at(viewID).contains(assetName))
                return Base::AssetHandle<TAsset>::Cast(m_ViewAssets.at(viewID).at(assetName).Handle);
            else {
                std::string msg = "Asset not found: \"" + assetName.ToString(true) + "\"";
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
