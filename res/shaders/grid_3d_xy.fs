#version 460 core

layout(location = 2) uniform float near;
layout(location = 3) uniform float far;

layout(location = 0) in vec3 near_point;
layout(location = 1) in vec3 far_point;
layout(location = 2) in mat4 view0;
layout(location = 6) in mat4 proj0;

out vec4 frag_color;

float compute_depth(vec3 pos) {
    vec4 clip_space_pos = proj0 * view0 * vec4(pos, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}

float compute_z(float depth) {
    return ((-2 * near * far) / (depth * (far - near) - far - near));
}

void main() {
    float t = -near_point.z / (far_point.z - near_point.z);
    vec3 frag_pos = near_point + t * (far_point - near_point);
    if(t > 0) {
        float depth = compute_depth(frag_pos);
        gl_FragDepth = (depth + 1.0) * 0.5;
        if(abs(frag_pos.y) < 0.0625) {
            frag_color = vec4(1.0, 0.25, 0.25, 1.0);
        } else if(abs(frag_pos.x) < 0.0625) {
            frag_color = vec4(0.25, 0.25, 1.0, 1.0);
        } else if(mod(frag_pos.x + 0.0625, 32) < 0.125 || mod(frag_pos.y + 0.0625, 32) < 0.125) {
            frag_color = vec4(0.25, 1.0, 0.25, 1.0 - (compute_z(depth) / 128));
        } else if(fract(frag_pos.x) < 0.0625 || fract(frag_pos.y) < 0.0625) {
            frag_color = vec4(0.25, 0.25, 0.25, 1.0 - (compute_z(depth) / 64));
        } else discard;
    } else discard;
}