#version 460 core
layout(location = 2) uniform vec4 color;
layout(binding = 0) uniform sampler2D text_texture;

in vec2 tex_coord;

out vec4 frag_color;

void main() {
    ivec2 i_tex_coord = ivec2(tex_coord);
    float alpha = texelFetch(text_texture, i_tex_coord, 0).a;
    if(alpha != 0.0) frag_color = color;
}