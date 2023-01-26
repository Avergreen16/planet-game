#version 460 core
layout(location = 2) uniform vec4 color;

out vec4 frag_color;

void main() {
    frag_color = color;
}