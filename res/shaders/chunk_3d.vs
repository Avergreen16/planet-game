#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec2 tile_coord;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform vec3 offset;
layout(location = 3) uniform vec3 light_dir;

out float col;
//out vec3 norm;
//out vec3 p;
out vec2 tex;
out vec2 tile_c;

float global_light = 0.3;

void main() {
    vec3 norm_light = normalize(light_dir);
    float weight = max(dot(vec3(0, 0, 1), norm_light), 0.0);
    col = ((dot(normal, norm_light) + 1) * 0.5 * 0.65 + 0.35) * weight + global_light * (1 - weight);
    //col = (dot(normal, vec3(0, 0, 1)) + 1) * 0.5;

    gl_Position = proj * (view * vec4(pos + offset * 16, 1.0));

    tex = tex_coord;
    tile_c = tile_coord;
}