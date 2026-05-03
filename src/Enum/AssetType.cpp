#include "BeatEngine/Enum/AssetType.h"
#include <map>

static std::map<AssetType, std::string> typeMap = {
    { AssetType::AudioStream, "AudioStream" },
    { AssetType::ComputeShader, "ComputeShader" },
    { AssetType::FragmentShader, "FragmentShader" },
    { AssetType::VertexShader, "VertexShader" },
    { AssetType::Font, "Font" },
    { AssetType::Texture, "Texture" },
    { AssetType::Sound, "Sound" }
};

std::string AssetTypeUtils::TypeToString(AssetType type) {
    return typeMap.at(type);    
}

AssetType AssetTypeUtils::StringToType(std::string typeString) {
    for (const auto& [type, typeStr] : typeMap) {
        if (typeStr == typeString)
            return type;
    }
    return AssetType::None;
}

std::map<AssetType, std::string> AssetTypeUtils::GetMap() {
    return typeMap;
}
