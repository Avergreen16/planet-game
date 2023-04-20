#version 460 core

layout(binding = 0) uniform sampler2D active_texture;

in float col;
//in vec3 norm;
//in vec3 p;
in vec2 tex;
in vec2 tile_c;

out vec4 frag_color;

const vec2 tex_factor = vec2(1.0 / 3, 0.5);
const float scale_factor = 1;

void main() {
    vec2 new_tex = tex / scale_factor;
    vec2 x = dFdx(new_tex * tex_factor);
    vec2 y = dFdy(new_tex * tex_factor);
    vec2 frac = fract(new_tex);
    frag_color = textureGrad(active_texture, (frac + tile_c) * tex_factor, x, y);
    frag_color = vec4(frag_color.xyz * col, frag_color.w);

    /*monoplanar
    if(norm.z > norm.y) {
        if(norm.z > norm.x) {
            frag_color = texture(active_texture, p.xy);
        } else {
            frag_color = texture(active_texture, p.yz);
        }
    } else {
        if(norm.y > norm.x) {
            frag_color = texture(active_texture, p.xz);
        } else {
            frag_color = texture(active_texture, p.yz);
        }
    }

    frag_color = vec4(frag_color.xyz * col, frag_color.w);*/

    /* biplanar
    if(norm.x < norm.y) {
        if(norm.z < norm.x) {
            vec2 n = norm.xy / (norm.x + norm.y);
            frag_color = texture(active_texture, p.yz) * n.x + texture(active_texture, p.xz) * n.y;
        } else {
            vec2 n = norm.yz / (norm.y + norm.z);
            frag_color = texture(active_texture, p.xz) * n.x + texture(active_texture, p.xy) * n.y;
        }
    } else {
        if(norm.z < norm.y) {
            vec2 n = norm.xy / (norm.x + norm.y);
            frag_color = texture(active_texture, p.yz) * n.x + texture(active_texture, p.xz) * n.y;
        } else {
            vec2 n = norm.xz / (norm.x + norm.z);
            frag_color = texture(active_texture, p.yz) * n.x + texture(active_texture, p.xy) * n.y;
        }
    }
    frag_color = vec4(frag_color.xyz * col, frag_color.w);*/

    /* triplanar
    vec4 x_col = texture(active_texture, p.yz);
    vec4 y_col = texture(active_texture, p.xz);
    vec4 z_col = texture(active_texture, p.xy);
    frag_color = vec4(((x_col * norm.x + y_col * norm.y + z_col * norm.z) * col).xyz, 1.0);*/
}