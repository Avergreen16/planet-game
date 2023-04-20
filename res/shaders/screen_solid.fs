#version 460 core

layout(binding = 0) uniform sampler2D buf;

layout(location = 0) uniform vec4 color;

in vec2 tex;

out vec4 frag_color;

void main() {
    vec4 tex_color = texture(buf, tex);
    float new_a = (tex_color.x + tex_color.y + tex_color.z) / 3 * tex_color.w;
    frag_color = vec4(tex_color.xyz * (new_a) + color.xyz * (1 - new_a), 1.0);
}