#version 460 core
layout(location = 2) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    vec4 base_color = texelFetch(active_texture, ivec2(tex_coords), 0);
    frag_color = vec4((base_color.rgb * 0.75 + vec3(0.25, 1.0, 1.0) * 0.25), 0.5 * base_color.a);
}