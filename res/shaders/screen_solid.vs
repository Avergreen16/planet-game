#version 460 core

vec3 grid_plane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, 1, 0), vec3(-1, -1, 0),
    vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0)
);

out vec2 tex;

void main() {
    vec3 vertex = grid_plane[gl_VertexID].xyz;
    gl_Position = vec4(vertex, 1.0);

    tex = (vertex.xy + 1) * 0.5;
}