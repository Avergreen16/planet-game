#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform vec3 offset;

out vec3 col;

void main() {
    col = color;
    gl_Position = proj * (view * vec4(pos + offset * 16, 1.0));
}