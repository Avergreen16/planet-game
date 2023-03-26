#version 460 core
layout(location = 0) uniform vec3 pos;

layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 proj;

void main() {
    gl_Position = proj * view * vec4(pos, 1.0);
}