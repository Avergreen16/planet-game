#version 460 core

vec3 grid_plane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, 1, 0), vec3(-1, -1, 0),
    vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0)
);

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;

layout(location = 0) out vec3 near_point;
layout(location = 1) out vec3 far_point;
layout(location = 2) out mat4 view0;
layout(location = 6) out mat4 proj0;

vec3 unproject_point(vec3 point, mat4 view_mat, mat4 proj_mat) {
    vec4 p = inverse(view_mat) * (inverse(proj_mat) * vec4(point, 1.0));
    return p.xyz / p.w;
}

void main() {
    vec3 p = grid_plane[gl_VertexID].xyz;
    near_point = unproject_point(vec3(p.xy, -1.0), view, proj);
    far_point = unproject_point(vec3(p.xy, 1.0), view, proj);
    view0 = view;
    proj0 = proj;
    gl_Position = vec4(p, 1.0);
}