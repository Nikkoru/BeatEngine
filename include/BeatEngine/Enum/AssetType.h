#pragma once

#include <cstdint>
#include <map>
#include <string>

enum class AssetType : uint8_t {
    None = 0,
	Texture,
	Sound,
	AudioStream,
    VertexShader,
    FragmentShader,
    ComputeShader,
	Font
};

namespace AssetTypeUtils {
    std::string TypeToString(AssetType type);
    AssetType StringToType(std::string typeString);
    std::map<AssetType, std::string> GetMap();
}
