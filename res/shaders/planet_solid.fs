#version 460 core
layout(location = 4) uniform vec3 color;

in vec3 frag_norm;
in float light;

out vec4 frag_color;

void main() {
    frag_color = vec4(color * (max(light, 0.0) * 0.875 + 0.125), 1.0);
}