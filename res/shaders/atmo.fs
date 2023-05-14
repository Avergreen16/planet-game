#version 460 core

layout(binding = 0) uniform sampler2D depth_tex;
layout(binding = 1) uniform sampler2D color_tex;

layout(location = 4) uniform vec3 player_pos;
layout(location = 5) uniform vec2 screen_size;
layout(location = 6) uniform vec3 sun_dir;

in vec3 frag_pos;
flat in float r;
flat in vec3 center_pos;
flat in mat4 inv_proj;

out vec4 frag_color;

vec3 color = vec3(0.35, 0.75, 1.0);
float r_planet = r * 0.9;
float scale = 1;
float max_chord_length = 2 * sqrt(r * r - r_planet * r_planet);

float thickness = r - r_planet;

void main() {
    vec3 dir = normalize(frag_pos - player_pos);

    float t = dot(center_pos - player_pos, dir);
    vec3 p = player_pos + dir * t;
    float y;
    float d = length(center_pos - p);
    if(t > 0) y = d;
    else {
        y = length(center_pos - player_pos);
        t = 0;
    }

    vec2 uv = gl_FragCoord.xy / screen_size;
    float depth = texture(depth_tex, uv).x;
    vec3 ndc = vec3(uv, depth) * 2.0 - 1.0;
    vec4 view_pos = inv_proj * vec4(ndc, 1.0);
    vec3 pers_div = view_pos.xyz / view_pos.w;
    float max_depth = length(pers_div.xyz);

    float atmo_depth = length(frag_pos - player_pos);

    float alpha = 0;

    float x = sqrt(r * r - y * y);

    float t1 = max(t - x, 0.0);
    float t2 = min(t + x, max_depth);

    float diff = t2 - t1;
    
    
    if(depth == 1.0) alpha = clamp(1.0 - (y - r_planet) / thickness, 0.0, 1.0);
    else {
        alpha = diff / thickness * 0.2;
    }

    vec3 up = normalize((player_pos + dir * min(t, t2) - center_pos));
    float dot_sun = dot(sun_dir, up) * 0.4 + 0.6;
    alpha = mix(0, alpha, dot_sun);


    float sun_bloom = 0.0;

    if(max_depth > atmo_depth) {
        float specular = pow((max(0.0, dot(dir, sun_dir)) + 0.5) / 1.5, 128);
        float fresnel = 1.0 - clamp(dot(-dir, normalize(center_pos - p)), 0.0, 1.0);
        fresnel *= fresnel;

        sun_bloom = (y - r_planet) / (thickness * 3.0);
        sun_bloom = 1.0 - clamp(sun_bloom, 0.0, 1.0);
        sun_bloom = pow(sun_bloom, 3);
        sun_bloom *= specular * fresnel;   
    }

    vec3 yellow = vec3(1.0, 1.0, 0.0);
    vec3 sunset = vec3(1.0, 0.5, 0.0);

    float blend = 0.0;
    if(alpha != 0.0) blend = pow(max(dot(dir * (1.0 - dot(dir, up)), sun_dir), 0.0), 1.5);

    frag_color = vec4(mix(color, sunset, blend) * alpha * (1 - sun_bloom) + yellow * sun_bloom, alpha + sun_bloom);
}