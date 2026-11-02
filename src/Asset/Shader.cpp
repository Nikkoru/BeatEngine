#include "BeatEngine/Asset/Shader.h"

std::map<Shader::Type, std::string> Shader::TypeUtils::TypeStrings = {
    {Shader::Type::Vertex, "Vertex"},
    {Shader::Type::Fragment, "Fragment"},
    {Shader::Type::Compute, "Compute"}
};

std::string Shader::TypeUtils::ToString(Type type) {
    return TypeStrings.at(type);
}

const Shader::SSBO& Shader::GetSSBO(const std::string& name) const {
    return m_SSBOs.at(name);
}

bool Shader::ContainsSSBO(const std::string& name) const {
    return m_SSBOs.contains(name);
}
