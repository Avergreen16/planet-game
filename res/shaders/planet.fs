#version 460 core

layout(binding = 0) uniform sampler2D surface_texture;

in vec3 frag_norm;
in vec2 frag_tex;
in float light;

out vec4 frag_color;

void main() {
    frag_color = vec4(texture(surface_texture, frag_tex).xyz * (max(light, 0.0) * 0.875 + 0.125), 1.0);
}