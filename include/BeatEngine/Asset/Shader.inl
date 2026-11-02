#include "BeatEngine/Asset/Shader.h"

template<typename TStruct>
void Shader::AddSSBO(TStruct& obj, const std::string& name, uint32_t binding) {
    m_SSBOs[name] = SSBO{
        .data = &obj,
        .size = sizeof(obj),
        .binding = binding
    };

    NewSSBO();
}
