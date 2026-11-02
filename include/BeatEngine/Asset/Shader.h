#pragma once

#include "BeatEngine/Asset/ShaderData.h"
#include "BeatEngine/Base/Asset.h"

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <map>

class Shader : public Base::Asset {
public:
    enum class Type {
        Vertex,
        Fragment,
        Compute,
    };
    struct TypeUtils {
        static std::map<Type, std::string> TypeStrings;
        static std::string ToString(Type type);
    };
    struct SSBO {
        void* data;
        size_t size;
        uint32_t binding = -1;
    };
protected:
    std::map<Type, std::shared_ptr<ShaderData>> m_Data;
    std::unordered_map<std::string, SSBO> m_SSBOs;
public:
    Shader() = default;
    virtual ~Shader() override = default;
public:
    template<typename TStruct>
    void AddSSBO(TStruct& obj, const std::string& name, uint32_t binding = -1);
    const SSBO& GetSSBO(const std::string& name) const;
    bool ContainsSSBO(const std::string& name) const;
protected:
    virtual void NewSSBO() = 0;
};

#include "BeatEngine/Asset/Shader.inl"
