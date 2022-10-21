#include "global.cpp"
#include "core.cpp"
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <thread>

time_t get_time() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

// vertex shaders

const char* vs_chunk = R"""(
#version 460 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec2 lim;

layout(location = 0) uniform mat4 pv_mat;

out vec2 tex_coord;
out vec2 limit;

void main() {
    tex_coord = tex;
    limit = lim;
    gl_Position = pv_mat * vec4(pos, 1.0);
}
)""";

// player shaders

const char* fs_chunk = R"""(
#version 460 core
layout(location = 0) out vec4 frag_color0;
layout(location = 1) out vec4 frag_color1;

in vec2 tex_coord;
in vec2 limit;
layout(location = 1) uniform sampler2D texture_input;

void main() {
    frag_color0 = texelFetch(texture_input, ivec2(min(tex_coord.x, limit.x), min(tex_coord.y, limit.y)), 0);
    if(frag_color0.w == 0.0) discard;
    frag_color1 = vec4(0.0, 0.0, 0.0, 1.0);
} 
)""";

const char* vs_player = R"""(
#version 460 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec2 lim;

layout(location = 0) uniform mat4 pv_mat;
layout(location = 1) uniform mat4 trans_mat;
layout(location = 2) uniform ivec2 active_tex;

out vec2 tex_coord;
out vec2 limit;

void main() {
    tex_coord = tex + active_tex * 32;
    limit = lim + active_tex * 32;
    gl_Position = pv_mat * (trans_mat * vec4(pos, 1.0));
}
)""";

const char* fs_player = R"""(
#version 460 core
layout(location = 0) out vec4 frag_color0;
layout(location = 1) out vec4 frag_color1;

in vec2 tex_coord;
in vec2 limit;
layout(location = 3) uniform sampler2D texture_input;

void main() {
    frag_color0 = texelFetch(texture_input, ivec2(min(tex_coord.x, limit.x), min(tex_coord.y, limit.y)), 0);
    frag_color1 = texelFetch(texture_input, ivec2(min(tex_coord.x, limit.x), min(tex_coord.y, limit.y) + 128), 0);
    if(frag_color0.w == 0.0) discard;
} 
)""";

const char* vs_screen = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

out vec2 tex_coord;

void main() {
    tex_coord = pos;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)""";

const char* fs_screen = R"""(
#version 460 core
out vec4 frag_color;

in vec2 tex_coord;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex_depth;
layout(location = 2) uniform sampler2D shadowmap0;
layout(location = 3) uniform sampler2D shadowmap1;

layout(location = 4) uniform mat4 inverse_matrix;
layout(location = 5) uniform mat4 shadow_matrix0;
layout(location = 6) uniform mat4 shadow_matrix1;

layout(location = 7) uniform ivec2 screen_size;
layout(location = 8) uniform vec3 sun_pos;

layout(location = 9) uniform sampler2D normal_map;

void main() {
    vec4 color = texture(tex, (tex_coord + 1) / 2);
    vec2 xy_pos = vec2(gl_FragCoord.x / screen_size.x, gl_FragCoord.y / screen_size.y);


    frag_color = color;
    vec4 normal = texture(normal_map, (tex_coord + 1) / 2);

    
    vec4 world_space_pos = inverse_matrix * vec4(xy_pos.x * 2 - 1, xy_pos.y * 2 - 1, texture(tex_depth, xy_pos).x * 2 - 1, 1.0);
    vec4 shadowmap0_pos = shadow_matrix0 * world_space_pos;
    vec4 shadowmap1_pos = shadow_matrix1 * world_space_pos;

    vec4 mult = vec4(0.4, 0.4, 0.3, 1.0);

    float shadow_depth0 = texture(shadowmap0, (shadowmap0_pos.xy + 1) / 2).x;
    if(shadowmap0_pos.z <= shadow_depth0 * 2 - 1) {
        if(!(normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)) {
            frag_color *= vec4(0.4, 0.4, 0.3, 1.0) + vec4(0.6, 0.5, 0.3, 1.0) * (max(dot(normalize(sun_pos), normalize(normal.xyz - vec3(0.5, 0.5, 0.5))), 0.0));

            return;
        } else {
            mult += vec4(0.4, 0.4, 0.3, 1.0);
        }
    }

    float shadow_depth1 = texture(shadowmap1, (shadowmap1_pos.xy + 1) / 2).x;
    if(shadowmap1_pos.z <= shadow_depth1 * 2 - 1) {
        mult += vec4(0.2, 0.1, 0.0, 0.0);
    }

    frag_color *= mult;
}
)""";

std::array<glm::vec2, 6> screen_vertices = {
    glm::vec2{-1, -1},
    glm::vec2{1, -1},
    glm::vec2{-1, 1},
    glm::vec2{-1, 1},
    glm::vec2{1, -1},
    glm::vec2{1, 1}
};

std::array<Vertex, 30> box_vertices {
    Vertex({0, 0, 0}, {0, 0}, {15, 11}),
    Vertex({1, 0, 0}, {16, 0}, {15, 11}),
    Vertex({1, 0, 0.75}, {16, 12}, {15, 11}),
    Vertex({0, 0, 0}, {0, 0}, {15, 11}),
    Vertex({1, 0, 0.75}, {16, 12}, {15, 11}),
    Vertex({0, 0, 0.75}, {0, 12}, {15, 11}),

    Vertex({1, 0, 0}, {0, 0}, {15, 11}),
    Vertex({1, 1, 0}, {16, 0}, {15, 11}),
    Vertex({1, 1, 0.75}, {16, 12}, {15, 11}),
    Vertex({1, 0, 0}, {0, 0}, {15, 11}),
    Vertex({1, 1, 0.75}, {16, 12}, {15, 11}),
    Vertex({1, 0, 0.75}, {0, 12}, {15, 11}),

    Vertex({1, 1, 0}, {0, 0}, {15, 11}),
    Vertex({0, 1, 0}, {16, 0}, {15, 11}),
    Vertex({0, 1, 0.75}, {16, 12}, {15, 11}),
    Vertex({1, 1, 0}, {0, 0}, {15, 11}),
    Vertex({0, 1, 0.75}, {16, 12}, {15, 11}),
    Vertex({1, 1, 0.75}, {0, 12}, {15, 11}),

    Vertex({0, 1, 0}, {0, 0}, {15, 11}),
    Vertex({0, 0, 0}, {16, 0}, {15, 11}),
    Vertex({0, 0, 0.75}, {16, 12}, {15, 11}),
    Vertex({0, 1, 0}, {0, 0}, {15, 11}),
    Vertex({0, 0, 0.75}, {16, 12}, {15, 11}),
    Vertex({0, 1, 0.75}, {0, 12}, {15, 11}),

    Vertex({0, 0, 0.75}, {0, 12}, {15, 27}),
    Vertex({1, 0, 0.75}, {16, 12}, {15, 27}),
    Vertex({1, 1, 0.75}, {16, 28}, {15, 27}),
    Vertex({0, 0, 0.75}, {0, 12}, {15, 27}),
    Vertex({1, 1, 0.75}, {16, 28}, {15, 27}),
    Vertex({0, 1, 0.75}, {0, 28}, {15, 27}),
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS) {
        if(core.keymap.contains(key)) {
            core.keymap[key] = true;
        }
    } else if(action == GLFW_RELEASE) {
        if(core.keymap.contains(key)) {
            core.keymap[key] = false;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    core.mouse_pos = glm::ivec2(x_pos, y_pos);
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    if(y_offset > 0 && core.scale < 80) {
        core.scale += 16;
        core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
        core.reload_active_chunks = true;
    } else if(y_offset < 0 && core.scale > 16) {
        core.scale -= 16;
        core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
        core.reload_active_chunks = true;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    core.screen_size.x = width + 1 * (width % 2 == 1);
    core.screen_size.y = height + 1 * (height % 2 == 1);

    core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
    core.reload_active_chunks = true;

    core.framebuffer.resize({core.screen_size.x, core.screen_size.y});
    //core.fb_light.resize({core.screen_size.x, core.screen_size.y});
}

int main() {
    bool game_running = true;
    glm::ivec2 screen_size(800, 600);
    int scale = 48;

    stbi_set_flip_vertically_on_load(true);
    // init glfw
    if(glfwInit() == GLFW_FALSE) {
        std::cout << "ERROR: GLFW failed to load.\n";
        return -1;
    }

    // set version and create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(screen_size.x, screen_size.y, "This is a test.", NULL, NULL);
    if(window == NULL) {
        std::cout << "ERROR: Window creation failed.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // init glad
    if(!gladLoadGL()) {
        std::cout << "ERROR: GLAD failed to load.\n";
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, 800, 600);


    // set callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    std::vector<const char*> addresses = {
        "res\\tiles.png",
        "res\\player_spritesheet.png",
        "res\\box.png"
    };
    std::vector<const char*> shader_vec = {
        vs_chunk, fs_chunk, vs_player, fs_player, vs_screen, fs_screen
    };

    // core
    Core core(game_running, screen_size, scale);
    core.create_textures(addresses);
    core.create_shaders(shader_vec);
    core.init(glm::dvec2{0, 0});
    glfwSetWindowUserPointer(window, (void*)&core);

    time_t last_time = get_time();
    uint32_t delta_time;

    Buffer buffer;
    buffer.init();
    buffer.set_data(player_vertices.data(), 6 * sizeof(Vertex));
    buffer.set_attrib(0, 3, 7 * sizeof(float), 0);
    buffer.set_attrib(1, 2, 7 * sizeof(float), 3 * sizeof(float));
    buffer.set_attrib(2, 2, 7 * sizeof(float), 5 * sizeof(float));

    // framebuffer

    core.buffers.resize(2);

    core.buffers[0].init();
    core.buffers[0].set_data(screen_vertices.data(), sizeof(glm::vec3) * screen_vertices.size());
    core.buffers[0].set_attrib(0, 2, 2 * sizeof(float), 0);

    core.buffers[1].init();
    core.buffers[1].set_data(box_vertices.data(), sizeof(Vertex) * 30);
    core.buffers[1].set_attrib(0, 3, 7 * sizeof(float), 0);
    core.buffers[1].set_attrib(1, 2, 7 * sizeof(float), 3 * sizeof(float));
    core.buffers[1].set_attrib(2, 2, 7 * sizeof(float), 5 * sizeof(float));

    //

    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LEQUAL);

    int fps = 100;
    int delta_lim = 1000000000 / fps;

    while(game_running) {
        glEnable(GL_DEPTH_TEST);
        glfwPollEvents();

        // math
        time_t current_time = get_time();
        delta_time = current_time - last_time;

        if(delta_time >= delta_lim) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            last_time = current_time;

            core.math(delta_time);
            core.render();            

            glfwSwapBuffers(window);

            game_running = !glfwWindowShouldClose(window);
        }
    }

    if(core.active_chunk_thread.joinable()) core.active_chunk_thread.join();
    if(core.chunk_update_thread.joinable()) core.chunk_update_thread.join();

    glfwTerminate();
    std::cout << "Successfully terminated!\n";

    return 0;
}