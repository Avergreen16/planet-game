#version 460 core
layout(binding = 0) buffer pos_buffer {
    ivec2 limit[];
};
layout(location = 2) uniform vec3 color;
layout(location = 3) uniform int border_width;

in vec2 pos1;

out vec4 frag_color;

void main() {
    ivec2 lim = limit[gl_PrimitiveID / 2];
    if(pos1.x < border_width || pos1.y < border_width || pos1.x > lim.x - border_width || pos1.y > lim.y - border_width) {
        frag_color = vec4(color, 1.0);
    } else {
        discard;
    }
}