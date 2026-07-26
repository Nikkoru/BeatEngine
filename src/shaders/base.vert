#version 460
#extension GL_GOOGLE_include_directive : require

#include "base_pcs.glsl"

layout (location = 0) in vec2 inPos;
layout (location = 5) in vec2 inUV;
layout (location = 6) in vec4 inColor;
layout (location = 1) out vec4 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) flat out uint textureID;
layout (location = 4) flat out uint shaderID;


void main() {
    DrawCommand command = pcs.drawBuffer.command;

    gl_Position = command.projection * command.transform * vec4(inPos, 0.0f, 1.0f);
    gl_Position.z = 0.0f;

    gl_PointSize = 1.0f;
    outUV = inUV;
    outColor = inColor;
    textureID = command.textureID;
    shaderID = command.shaderID;
}
