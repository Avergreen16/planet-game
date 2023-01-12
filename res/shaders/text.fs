#version 460 core
layout(binding = 0) buffer color_buffer {
    vec4 colors[];
};
layout(binding = 0) uniform sampler2D text_texture;

in vec2 tex_coord;

out vec4 frag_color;

void main() {
    float alpha = texelFetch(text_texture, ivec2(tex_coord), 0).a;
    if(alpha != 0.0) frag_color = colors[gl_PrimitiveID / 2];
    else discard;
}