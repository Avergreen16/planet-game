#version 460 core
layout(binding = 0) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = texelFetch(active_texture, ivec2(tex_coords), 0);
}