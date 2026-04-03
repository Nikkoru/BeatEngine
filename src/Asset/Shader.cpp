#include "BeatEngine/Asset/Shader.h"

std::map<Shader::Type, std::string> Shader::TypeUtils::TypeStrings = {
    {Shader::Type::Vertex, "Vertex"},
    {Shader::Type::Fragment, "Fragment"}
};

std::string Shader::TypeUtils::ToString(Type type) {
    return TypeStrings.at(type);
}

void Shader::SetType(Type type) {
    m_Type = type;    
}
