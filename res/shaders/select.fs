#version 460 core
layout(location = 2) uniform float alpha;

out vec4 frag_color;

void main() {
    frag_color = vec4(0.25, 1.0, 1.0, alpha);
}