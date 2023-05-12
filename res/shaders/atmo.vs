#version 460 core

vec3 vertices[] = {
    vec3(-1, -1, -1),
    vec3(1, -1, -1),
    vec3(-1, 1, -1),
    vec3(1, 1, -1),
    vec3(-1, -1, 1),
    vec3(1, -1, 1),
    vec3(-1, 1, 1),
    vec3(1, 1, 1),
};

int indices[] = {
    0, 2, 4,   4, 2, 6,
    3, 1, 7,   7, 1, 5,
    1, 0, 5,   5, 0, 4,
    2, 3, 6,   6, 3, 7,
    3, 2, 1,   1, 2, 0,
    5, 4, 7,   7, 4, 6
};

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform vec3 pos;
layout(location = 3) uniform float radius;

out vec3 frag_pos;
flat out float r;
flat out vec3 center_pos;
flat out mat4 inv_proj;

void main() {
    vec3 v = vertices[indices[gl_VertexID]] * radius + pos;

    frag_pos = v;
    r = radius;
    center_pos = pos;

    inv_proj = inverse(proj);

    gl_Position = proj * view * vec4(v, 1.0);
}