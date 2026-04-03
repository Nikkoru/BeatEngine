#pragma once

#include "BeatEngine/Asset/ShaderData.h"
#include "BeatEngine/Base/Asset.h"

#include <filesystem>
#include <map>

class Shader : public Base::Asset {
public:
    enum class Type {
        Vertex,
        Fragment,
    };
    struct TypeUtils {
        static std::map<Type, std::string> TypeStrings;
        static std::string ToString(Type type);
    };
protected:
    std::shared_ptr<ShaderData> m_Data;
    Type m_Type;
public:
    Shader() = default;
    ~Shader() override = default;
public:
    virtual bool GetFileContents(const std::filesystem::path path) = 0;
    void SetType(Type type);
};
