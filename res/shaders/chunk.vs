#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

layout(location = 0) uniform mat3 pos_matrix;
layout(location = 1) uniform mat3 cam_matrix;

out vec2 tex_coords;

void main() {
    tex_coords = tex;
    gl_Position = vec4((cam_matrix * pos_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}