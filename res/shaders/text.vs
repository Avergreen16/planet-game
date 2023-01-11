#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

layout(location = 0) uniform mat3 view_mat;
layout(location = 1) uniform mat3 trans_mat;

out vec2 tex_coord;

void main() {
    tex_coord = tex;
    gl_Position = vec4((view_mat * trans_mat * vec3(pos, 1.0)).xy, 0.5, 1.0);
}