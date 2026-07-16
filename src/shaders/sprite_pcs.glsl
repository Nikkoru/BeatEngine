#extension GL_EXT_scalar_block_layout: require

struct SpriteDrawCommand {

};

layout (buffer_reference, scalar) readonly buffer SpriteDrawBuffer  {
    SpriteDrawCommand commands[];
};

layout (push_constant) uniform constants {
    mat4 viewProj;
    SpriteDrawBuffer drawBuffer;
} pcs;
