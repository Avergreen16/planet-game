#version 460 core
layout(location = 2) uniform vec4 color;

in vec2 coord;

out vec4 frag_color;

void main() {
    if(sqrt(coord.x * coord.x + coord.y * coord.y) > 1.0) discard;
    frag_color = color;
}