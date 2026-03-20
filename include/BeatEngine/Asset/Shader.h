#pragma once

#include "BeatEngine/Base/Asset.h"

#include <filesystem>
#include <string_view>
#include <vector>

class Shader : public Base::Asset {
public:
    ~Shader() override = default;
public:
    enum class Type {
        Vertex,
        Fragment
    };
public:
    bool LoadFromFile(const std::filesystem::path path, Type type);
public:
    virtual bool GetFileContents(const std::filesystem::path path, std::vector<char>& buffer);
    virtual bool Compile(std::string_view vertexData, std::string_view fragmentData) = 0;
};
