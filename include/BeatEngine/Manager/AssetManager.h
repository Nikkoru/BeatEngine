#pragma once

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

class GameContext;
class GameState;
class ImGuiMultiSelectIO;
class AssetManager {
	struct Slot {
		Base::AssetHandle<void> Handle;
		std::shared_ptr<Base::Asset> Asset;
        std::type_index Type{ typeid(nullptr) };

		Slot() = default;
		Slot(Base::AssetHandle<void> handle, std::shared_ptr<Base::Asset> asset, std::type_index type = typeid(nullptr)) : Handle(handle), Asset(asset), Type(type) {}
	};
public:
    AssetManager() : AssetManager(nullptr, nullptr) {}
    AssetManager(GameContext* context, GameState* state);
    ~AssetManager();
public:
    void SetContext(GameContext* context) { m_Context = context; }
    void SetState(GameState* state) { m_State = state; }

    void Init();
    void Uninit();
private:
    std::unordered_map<AssetType, fs::path> m_AssetsToLoad;
	std::unordered_map<String, Slot> m_GlobalAssets;
	std::unordered_map<std::type_index, std::unordered_map<String, Slot>> m_ViewAssets;
private:
	uint64_t m_AudioSampleRate = 48000;
    bool m_ShowAssetBrowser{ false };
private:
    GameContext* m_Context{ nullptr };
    GameState* m_State{ nullptr };
public:
	template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset> && !std::is_base_of_v<Shader, TAsset>)
	Base::AssetHandle<TAsset> Load(const fs::path& path, const std::type_index viewID = typeid(nullptr));
    Base::AssetHandle<Shader> LoadShader(const fs::path& path, Shader::Type type, const std::type_index viewID = typeid(nullptr));
	template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset>)
	Base::AssetHandle<TAsset> Get(const String& assetName, const std::type_index viewID = typeid(nullptr));
    bool Has(const String& name, const std::type_index viewID = typeid(nullptr));

    bool Preload(AssetType type, const fs::path& path, const std::type_index viewID = typeid(nullptr));

    void ShowImGuiDebugWindow();
    void ShowAssetBrowser();
private:
    void ApplySelections(ImGuiMultiSelectIO* io, std::vector<UID>& ids, std::vector<Slot>& totalAssets); 
};

#include "BeatEngine/Manager/AssetManager.inl"
