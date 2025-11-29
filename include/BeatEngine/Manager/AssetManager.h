#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <filesystem>
#include <unordered_map>
#include <cstdint>
#include <format>
#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Logger.h"

namespace fs = std::filesystem;

class AssetManager {

	struct Slot {
		Base::AssetHandle<void> Handle;
		std::shared_ptr<Base::Asset> Asset;

		Slot() = default;
		Slot(Base::AssetHandle<void> handle, std::shared_ptr<Base::Asset> asset) : Handle(handle), Asset(asset) {}
	};
private:
	std::unordered_map<std::string, Slot> m_GlobalAssets;
	std::unordered_map<std::type_index, std::unordered_map<std::string, Slot>> m_ViewAssets;
private:
	uint64_t m_AudioSampleRate = 48000;
public:
	template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset>)
	Base::AssetHandle<TAsset> Load(const fs::path& path, const std::type_index viewID = typeid(nullptr));
	template <typename TAsset>
		requires(std::is_base_of_v<Base::Asset, TAsset>)
	Base::AssetHandle<TAsset> Get(const std::string assetName, const std::type_index viewID = typeid(nullptr)) {
		if (viewID == typeid(nullptr)) {
			if (m_GlobalAssets.contains(assetName))
				return Base::AssetHandle<TAsset>::Cast(m_GlobalAssets.at(assetName).Handle);
			else {
				std::string msg = "Asset not found: \"" + assetName + "\"";
				Logger::GetInstance()->AddCritical(msg, typeid(AssetManager));
				THROW_RUNTIME_ERROR(msg);
			}
		}
		else {
			if (m_ViewAssets.contains(viewID)) {
				if (m_ViewAssets.at(viewID).contains(assetName))
					return Base::AssetHandle<TAsset>::Cast(m_GlobalAssets.at(assetName).Handle);
				else {
					std::string msg = "Asset not found: \"" + assetName + "\"";
					Logger::GetInstance()->AddCritical(msg, typeid(AssetManager));
					THROW_RUNTIME_ERROR(msg);
				}
			}
			else {
				std::string msg = std::format("View {} dosen't have assets", viewID.name());
				Logger::GetInstance()->AddCritical(msg, typeid(AssetManager));
				THROW_RUNTIME_ERROR(msg);
			}
		}
	}
};