#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Logger.h"

bool Shader::LoadFromFile(const std::filesystem::path path, Type type) {
    std::vector<char> shader;
    if (GetFileContents(path, shader)) {
        Logger::AddError(typeid(Shader), "Failed to open shader file");
    }

    switch (type) {
        case Type::Fragment:
            return Compile({}, shader.data());
        case Type::Vertex:
            return Compile(shader.data() ,{});
        default:
            Logger::AddError(typeid(Shader), "Invalid type of shader {}", type);
            return false;
    }
}
