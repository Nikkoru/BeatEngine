#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout: require

struct DrawCommand {
    mat4 projection;
    mat4 transform;
    vec2 padding;
    uint textureID;
    uint shaderID;
};

layout (scalar, buffer_reference, buffer_reference_align = 4) readonly buffer DrawBuffer {
    DrawCommand command;
};

layout (push_constant) uniform constants {
    DrawBuffer drawBuffer;
} pcs;
