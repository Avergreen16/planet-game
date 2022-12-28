#version 460 core
layout(location = 0) in vec2 pos;

layout(location = 0) uniform mat3 matrix;
layout(location = 1) uniform vec2 cam_pos;

out vec2 world_coord;
out ivec2 recentered;

void main() {
    world_coord = (matrix * vec3(pos, 1.0)).xy;
    recentered = ivec2(0, 0);

    if(abs(cam_pos.x) > 4096.0) {
        world_coord.x -= floor(cam_pos.x / 4096.0) * 4096.0;
        recentered.x = 1;
    }
    if(abs(cam_pos.y) > 4096.0) {
        world_coord.y -= floor(cam_pos.y / 4096.0) * 4096.0;
        recentered.y = 1;
    }

    gl_Position = vec4(pos, 0.5, 1.0);
}