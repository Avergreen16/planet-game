#version 460 core

layout(binding = 0) uniform sampler2D active_texture;

in float col;
//in vec3 norm;
//in vec3 p;
in vec2 tex;
in vec2 tile_c;

out vec4 frag_color;

const vec2 tex_factor = vec2(0.25, 0.25);
const float scale_factor = 2;

void main() {
    //frag_color = vec4(col, col, col, 1.0);
    vec2 new_tex = tex * scale_factor;
    vec2 x = dFdx(new_tex * tex_factor);
    vec2 y = dFdy(new_tex * tex_factor);
    vec2 frac = fract(new_tex);
    frag_color = textureGrad(active_texture, (frac + tile_c) * tex_factor, x, y);
    frag_color = vec4(frag_color.xyz * col, frag_color.w);

    if(tex.x == 0 && tex.y == 0) frag_color = vec4(0.0, 0.0, 1.0, 1.0);
}