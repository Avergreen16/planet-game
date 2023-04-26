#version 460 core

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3[8] pos;

int ids[24] = {0, 1, 1, 5, 4, 5, 0, 4, 2, 3, 3, 7, 6, 7, 2, 6, 0, 2, 1, 3, 4, 6, 5, 7};

void main() {
    gl_Position = proj * view * vec4(pos[ids[gl_VertexID]], 1.0);
}