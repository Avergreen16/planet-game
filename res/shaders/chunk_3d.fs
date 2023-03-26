#version 460 core

in vec3 col;

out vec4 frag_color;

void main() {
    frag_color = vec4(col, 1.0);
}