#version 460 core
layout(location = 0) uniform vec3 pos0;
layout(location = 1) uniform vec3 pos1;

layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 proj;

void main() {
    if(gl_VertexID == 0) gl_Position = proj * view * vec4(pos0, 1.0);
    else gl_Position = proj * view * vec4(pos1, 1.0);
}