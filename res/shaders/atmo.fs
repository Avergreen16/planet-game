#version 460 core

layout(binding = 0) uniform sampler2D depth_tex;
layout(binding = 1) uniform sampler2D color_tex;

layout(location = 4) uniform vec3 player_pos;
layout(location = 5) uniform vec2 screen_size;

in vec3 frag_pos;
flat in float r;
flat in vec3 center_pos;
flat in mat4 inv_proj;

out vec4 frag_color;

vec3 color = vec3(1.0, 1.0, 1.0);
vec3 wavelengths = vec3(700, 530, 440);
float scattering_strength = 10;

float scatter_r = pow(400 / wavelengths.r, 4) * scattering_strength;
float scatter_g = pow(400 / wavelengths.g, 4) * scattering_strength;
float scatter_b = pow(400 / wavelengths.b, 4) * scattering_strength;
vec3 scattering_coefficiants = vec3(scatter_r, scatter_g, scatter_b);

float ray_sphere(vec3 ray_origin, vec3 ray_dir, vec3 sphere_center, float sphere_radius) {
    float t = dot(sphere_center - ray_origin, ray_dir);
    vec3 p = ray_origin + ray_dir * t;
    float y = length(sphere_center - p);

    float x = sqrt(r * r - y * y);

    vec2 uv = gl_FragCoord.xy / screen_size;
    float depth = texture(depth_tex, uv).x;
    vec3 ndc = vec3(uv, depth) * 2.0 - 1.0;
    vec4 view_pos = inv_proj * vec4(ndc, 1.0);
    vec3 pers_div = view_pos.xyz / view_pos.w;
    float max_depth = length(pers_div.xyz);

    float t1 = max(t - x, 0.0);
    float t2 = t + x;

    return t2 - t1;
};

float planet_rad = r * 0.9;
float falloff = 10;
float density_at(vec3 pt) {
    float dist_from_surface = length(pt - center_pos) - planet_rad;
    float norm_height = dist_from_surface / (r - planet_rad);
    float local_density = exp(-norm_height * falloff) * (1 - norm_height);
    return local_density;
}

int num_pts_optical_depth = 5;
float optical_depth(vec3 ray_origin, vec3 ray_dir, float ray_length) {
    vec3 sample_pt = ray_origin;
    float step_size = ray_length / (num_pts_optical_depth - 1);
    float optical_depth = 0.0;

    for(int i = 0; i < num_pts_optical_depth; ++i) {
        float local_density = density_at(sample_pt);
        optical_depth += local_density;

        sample_pt += ray_dir * step_size;
    }

    return optical_depth * step_size;
}

vec3 sun_dir = normalize(vec3(1, 1, 1));
int num_pts = 5;
vec4 calculate_light(vec3 ray_origin, vec3 ray_dir, float ray_length) {
    vec3 scatter_pt = ray_origin;

    float step_size = ray_length / (num_pts - 1);
    vec3 in_scattered_light = vec3(0.0);
    for(int i = 0; i < num_pts; ++i) {
        float sun_ray_length = ray_sphere(scatter_pt, sun_dir, center_pos, r);
        float sun_ray_optical_depth = optical_depth(scatter_pt, sun_dir, sun_ray_length);
        float view_ray_optical_depth = optical_depth(scatter_pt, -ray_dir, step_size * i);

        vec3 transmitance = exp(-(sun_ray_optical_depth + view_ray_optical_depth) * scattering_coefficiants);
        float local_density = density_at(scatter_pt);

        in_scattered_light += local_density * transmitance * step_size * scattering_coefficiants;
        scatter_pt += ray_dir * step_size;
    }
    float optical = optical_depth(ray_origin, ray_dir, ray_length);
    //vec2 uv = gl_FragCoord.xy / screen_size;

    return vec4(in_scattered_light, optical);
};

void main() {
    vec3 dir = normalize(frag_pos - player_pos);

    float t = dot(center_pos - player_pos, dir);
    vec3 p = player_pos + dir * t;
    float y = length(center_pos - p);

    float x = sqrt(r * r - y * y);
    vec2 uv = gl_FragCoord.xy / screen_size;
    float depth = texture(depth_tex, uv).x;
    vec3 ndc = vec3(uv, depth) * 2.0 - 1.0;
    vec4 view_pos = inv_proj * vec4(ndc, 1.0);
    vec3 pers_div = view_pos.xyz / view_pos.w;
    float max_depth = length(pers_div.xyz);

    float t1 = max(t - x, 0.0);
    float t2 = min(t + x, max_depth);

    float diff = t2 - t1;

    vec3 pt = player_pos + dir * max(min(t, t2), 0.0);

    if(y <= r && diff > 0) {
        float epsilon = 0.001;
        vec3 pt_in_atmosphere = player_pos + dir * (t1 + epsilon);
        vec4 light = calculate_light(pt_in_atmosphere, dir, diff - epsilon * 2);
        //float light = max(dot(sun_dir, normalize(pt)), 0.0);

        frag_color = light;
    } else discard;
}