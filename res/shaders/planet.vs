#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;

layout(location = 0) uniform mat4 trans_mat;
layout(location = 1) uniform mat4 view_mat; 
layout(location = 2) uniform mat4 proj_mat;
layout(location = 3) uniform vec3 light_dir;
layout(location = 5) uniform int is_star;

out vec3 frag_norm;
out vec2 frag_tex;
out float light;

int prec = 6;

void main() {
    frag_norm = norm;
    frag_tex = tex;
    if(is_star == 1) light = 1.0;
    else light = dot(norm, light_dir);
    
    gl_Position = proj_mat * view_mat * trans_mat * vec4(pos, 1.0);
}