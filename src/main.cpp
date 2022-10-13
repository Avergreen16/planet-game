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

const char* vertex_shader_source = R"""(
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

const char* fragment_shader_source = R"""(
#version 460 core
out vec4 frag_color;

in vec2 tex_coord;
in vec2 limit;
layout(location = 1) uniform sampler2D texture_input;

void main() {
    frag_color = texelFetch(texture_input, ivec2(min(tex_coord.x, limit.x), min(tex_coord.y, limit.y)), 0);
    if(frag_color.w == 0.0) discard;
} 
)""";

const char* vss_player = R"""(
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

const char* fss_player = R"""(
#version 460 core
out vec4 frag_color;

in vec2 tex_coord;
in vec2 limit;
layout(location = 3) uniform sampler2D texture_input;

void main() {
    frag_color = texelFetch(texture_input, ivec2(min(tex_coord.x, limit.x), min(tex_coord.y, limit.y)), 0);
    if(frag_color.w == 0.0) discard;
} 
)""";

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
    glViewport(0, 0, core.screen_size.x, core.screen_size.y);

    core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
    core.reload_active_chunks = true;
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

    // texture address array
    std::array<const char*, 2> addresses = {
        "res\\tiles.png",
        "res\\player_spritesheet.png"
    };

    // core
    Core core(game_running, screen_size, scale);
    core.create_textures(addresses);
    core.init(glm::dvec2{0, 0});
    glfwSetWindowUserPointer(window, (void*)&core);

    // create shader
    Shader shader;
    shader.compile(vertex_shader_source, fragment_shader_source);

    Shader player_shader;
    player_shader.compile(vss_player, fss_player);

    time_t last_time = get_time();
    uint32_t delta_time;

    Buffer buffer;
    buffer.init();
    buffer.set_data(player_vertices.data(), 6 * sizeof(Vertex));
    buffer.set_attrib(0, 3, 7 * sizeof(float), 0);
    buffer.set_attrib(1, 2, 7 * sizeof(float), 3 * sizeof(float));
    buffer.set_attrib(2, 2, 7 * sizeof(float), 5 * sizeof(float));

    while(game_running) {
        glfwPollEvents();

        // math
        time_t current_time = get_time();
        delta_time = current_time - last_time;
        last_time = current_time;

        core.math(delta_time);

        //render
        glClearColor(0.2, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        //get matrix
        float w = float(screen_size.x / 2) / scale;
        float h = float(screen_size.y / 2) / scale;
        glm::mat4 pv_matrix = glm::ortho(-w, w, -h, h, 0.0f, 16.0f);
        pv_matrix *= core.camera.get_view_matrix();

        // draw
        shader.use();
        core.textures[0].bind(0, 1);

        // this is to stop too many chunks from being loaded into the gpu per frame and lagging the game
        int chunks_loaded_vertices = 0;

        for(uint64_t& key : core.active_chunks) {
            if(core.loaded_chunks.contains(key)) {
                Chunk& chunk = core.loaded_chunks[key];
                // chunk vertices are in buffer
                if(chunk.vertex_status == 2) { 
                    chunk.buffer.bind();

                    glUniformMatrix4fv(0, 1, GL_FALSE, &pv_matrix[0][0]);
                    glDrawArrays(GL_TRIANGLES, 0, chunk.vertex_count);
                // if vertices have been generated but not loaded
                } else if(chunk.vertex_status == 1 && chunks_loaded_vertices < 5) {
                    chunk.load_mesh();
                    ++chunks_loaded_vertices;
                // if vertices haven't been generated
                } else if(chunk.vertex_status == 0) {
                    chunk.vertex_status = 3; // stops the key from being inserted into the queue more that once
                    core.chunk_update_queue.push(key);
                }
            }
        }

        /*shader.use();
        core.textures[1].bind(0, 2);

        glm::mat4 transform = glm::translate(glm::vec3(core.player.position, 0));

        buffer.bind();
        glUniformMatrix4fv(0, 1, GL_FALSE, &pv_matrix[0][0]);
        glUniformMatrix4fv(1, 1, GL_FALSE, &transform[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);*/

        core.player.render(player_shader, pv_matrix);

        glfwSwapBuffers(window);

        game_running = !glfwWindowShouldClose(window);
    }

    if(core.active_chunk_thread.joinable()) core.active_chunk_thread.join();
    if(core.chunk_update_thread.joinable()) core.chunk_update_thread.join();

    glfwTerminate();
    std::cout << "Successfuly terminated!\n";

    return 0;
}