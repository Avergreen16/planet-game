#version 460 core

layout(location = 2) uniform int scale;

in vec2 world_coord;
flat in ivec2 recentered;

out vec4 frag_color;

void main() {
    float pixel_size = 1.0 / scale;

    ivec2 floor_coord = ivec2(floor(world_coord));
    
    float pixel_shift = pixel_size * int(scale >= 16);
    ivec2 shifted_floor_coord = ivec2(floor(world_coord + pixel_shift));

    float difference_x = world_coord.x - floor_coord.x;
    float difference_y = world_coord.y - floor_coord.y;
    
    if(shifted_floor_coord.y % 16 == 0 && world_coord.y - shifted_floor_coord.y < pixel_size) {
        if(recentered.y == 0 && shifted_floor_coord.y == 0) {
            frag_color = vec4(1.0, 0.25, 0.25, 0.5);
        } else if(shifted_floor_coord.y % 256 == 0) {
            frag_color = vec4(1.0, 1.0, 0.25, 0.5);
        } else {
            if(shifted_floor_coord.x == 0 && world_coord.x - shifted_floor_coord.x < pixel_size) {
                frag_color = vec4(0.25, 0.25, 1.0, 0.5);
            } else {
                frag_color = vec4(0.25, 1.0, 1.0, 0.5);
            }
        }
    } else if(shifted_floor_coord.x % 16 == 0 && world_coord.x - shifted_floor_coord.x < pixel_size) {
        if(recentered.x == 0 && shifted_floor_coord.x == 0) {
            frag_color = vec4(0.25, 0.25, 1.0, 0.5);
        } else if(shifted_floor_coord.x % 256 == 0) {
            frag_color = vec4(1.0, 1.0, 0.25, 0.5);
        } else {
            frag_color = vec4(0.25, 1.0, 1.0, 0.5);
        }
    } else if(scale >= 16 && (difference_x < pixel_size || difference_y < pixel_size)) {
        frag_color = vec4(0.25, 1.0, 1.0, 0.25);
    } else {
        discard;
    }
}