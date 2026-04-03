#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <filesystem>
#include <unordered_map>
#include <cstdint>

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Base/Asset.h"

namespace fs = std::filesystem;

class GameContext;
class GameState;
class AssetManager {
	struct Slot {
		Base::AssetHandle<void> Handle;
		std::shared_ptr<Base::Asset> Asset;

		Slot() = default;
		Slot(Base::AssetHandle<void> handle, std::shared_ptr<Base::Asset> asset) : Handle(handle), Asset(asset) {}
	};
public:
    AssetManager() : AssetManager(nullptr, nullptr) {}
    AssetManager(GameContext* context, GameState* state);
    ~AssetManager();
private:
	std::unordered_map<std::string, Slot> m_GlobalAssets;
	std::unordered_map<std::type_index, std::unordered_map<std::string, Slot>> m_ViewAssets;
private:
	uint64_t m_AudioSampleRate = 48000;
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
	Base::AssetHandle<TAsset> Get(const std::string assetName, const std::type_index viewID = typeid(nullptr));
    bool Has(std::string name, const std::type_index viewID = typeid(nullptr));

    void DrawImGuiDebug();
};

#include "BeatEngine/Manager/AssetManager.inl"
