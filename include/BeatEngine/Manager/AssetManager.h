#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <filesystem>
#include <unordered_map>
#include <cstdint>
#include <vector>

#include "BeatEngine/System/String.hpp"
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Enum/AssetType.h"

namespace fs = std::filesystem;

class AppContext;
class AppState;
class ImGuiMultiSelectIO;
class Renderer;
class AssetManager {
public:
	struct Slot {
		Base::AssetHandle<void> Handle;
		std::shared_ptr<Base::Asset> Asset;
        std::type_index Type{ typeid(nullptr) };

		Slot() = default;
		Slot(Base::AssetHandle<void> handle, std::shared_ptr<Base::Asset> asset, std::type_index type = typeid(nullptr)) : Handle(handle), Asset(asset), Type(type) {}
	};
    using Assets = std::unordered_map<std::type_index, std::vector<std::filesystem::path>>;
    using AssetLoadCallback = std::function<std::pair<Base::AssetHandle<void>, std::shared_ptr<Base::Asset>>(const fs::path&)>;
    using AssetUnloadCallback = std::function<void(const Base::AssetHandle<void>&)>;
public:
    AssetManager() : AssetManager(nullptr, nullptr) {}
    AssetManager(AppContext* context, AppState* state);
    ~AssetManager();
public:
    void SetContext(AppContext* context) { m_Context = context; }
    void SetState(AppState* state) { m_State = state; }
private:
    Assets m_AssetsToLoad;
	std::unordered_map<String, Slot> m_GlobalAssets;
	std::unordered_map<std::type_index, std::unordered_map<String, Slot>> m_ViewAssets;
    std::unordered_map<std::type_index, AssetLoadCallback> m_LoadCallbacks;
    std::unordered_map<std::type_index, AssetUnloadCallback> m_UnloadCallbacks;
private:
	uint64_t m_AudioSampleRate = 48000;
    bool m_ShowAssetBrowser{ false };
private:
    AppContext* m_Context{ nullptr };
    AppState* m_State{ nullptr };
public:
    template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset>)
    void SetLoadCallback(AssetLoadCallback callback);
    template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset>)
    void SetUnloadCallback(AssetUnloadCallback callback);

	template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset>)
	Base::AssetHandle<TAsset> Load(const fs::path& path, const std::type_index viewID = typeid(nullptr));
    
    void BulkLoad(const Assets& assets, const std::type_index& viewID = typeid(nullptr));
	template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset>)
	Base::AssetHandle<TAsset> Get(const String& assetName, const std::type_index viewID = typeid(nullptr));
    bool Has(const String& name, const std::type_index viewID = typeid(nullptr));

    void ShowImGuiDebugWindow(Renderer* const renderer);
    void ShowAssetBrowser(Renderer* const renderer);
private:
    void ApplySelections(ImGuiMultiSelectIO* io, std::vector<UID>& ids, std::vector<Slot>& totalAssets); 
    Base::AssetHandle<void> _DoLoad(std::type_index assetType, const fs::path& path, std::type_index viewID);
};

#include "BeatEngine/Manager/AssetManager.inl"
