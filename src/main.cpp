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

const char* vss_screen = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

out vec2 tex_coord;

void main() {
    tex_coord = pos;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)""";

const char* fss_screen = R"""(
#version 460 core
out vec4 frag_color;

in vec2 tex_coord;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform sampler2D tex_depth;
layout(location = 2) uniform sampler2D shadowmap;
layout(location = 3) uniform mat4 inverse_matrix;
layout(location = 4) uniform mat4 shadow_matrix;
layout(location = 5) uniform ivec2 screen_size;
layout(location = 6) uniform vec2 near_far;

void main() {
    vec4 color = texture(tex, (tex_coord + 1) / 2);
    vec2 xy_pos = vec2(gl_FragCoord.x / screen_size.x, gl_FragCoord.y / screen_size.y);
    vec4 position_in_shadowmap = shadow_matrix * inverse_matrix * vec4(xy_pos.x * 2 - 1, xy_pos.y * 2 - 1, texture(tex_depth, xy_pos).x * (near_far.y - near_far.x) + near_far.x, 1.0);
    if(position_in_shadowmap.x < -1.0 || position_in_shadowmap.x > 1.0 || position_in_shadowmap.y < -1.0 || position_in_shadowmap.y > 1.0 || position_in_shadowmap.z < -1.0 || position_in_shadowmap.z > 1.0) {
        frag_color = color * 0.3;
    } else {
        if(int(position_in_shadowmap.x * 80) % 2 == 1 || int(position_in_shadowmap.y * 80) % 2 == 1) {
            frag_color = vec4(1.0, 1.0, 1.0, 0.0);
        } else {
            frag_color = texture(shadowmap, (position_in_shadowmap.xy + 1) / 2);//color;
        }
    }
}
)""";
// position_in_shadowmap.x < -1.0 || position_in_shadowmap.x > 1.0 || position_in_shadowmap.y < -1.0 || position_in_shadowmap.y > 1.0 || position_in_shadowmap.z < -1.0 || position_in_shadowmap.z > 1.0

/*const char* vss_raycast = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
}
)""";

const char* fss_raycast = R"""(
#version 460 core
out vec4 frag_color;

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform ivec2 screen_size;
layout(location = 2) uniform ivec2 light_pos;
layout(location = 3) uniform int light_rad;

float pi = 3.14159265358979323846;

void main() {
    float num_rays = floor(light_rad * pi * 2);
    int pos = int((gl_FragCoord.x - 0.5) + ((gl_FragCoord.y - 0.5) * 128));
    if(pos < num_rays) {
        float theta = (pos / num_rays) * (pi * 2);
        vec2 step = vec2(cos(theta), sin(theta));

        float length = 0.0;

        bool stop = false;
        for(int i = 0; i < 256; i++) {
            if(stop == false && i < light_rad) {
                ivec2 step_pos = ivec2(floor(step.x * i + 0.5), floor(step.y * i + 0.5)) + light_pos;
                vec4 color = texelFetch(tex, step_pos, 0);
                if(color.x != 0.0) {
                    stop = true;
                } else {
                    length++;
                }
            }
        }

        frag_color = vec4(length / 256, 0.0, 0.0, 1.0);
    } else discard;
}
)""";

const char* vss_light = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
}
)""";

const char* fss_light = R"""(
#version 460 core
out vec4 frag_color;

layout(location = 0) uniform sampler2D ray_lengths;
layout(location = 1) uniform ivec2 light_pos;
layout(location = 2) uniform int light_rad;

float pi = 3.14159265358979323846;

void main() {
    vec2 rel_pos = gl_FragCoord.xy - vec2(0.49, 0.49) - light_pos;
    float squared_dist = rel_pos.x * rel_pos.x + rel_pos.y * rel_pos.y;
    if(squared_dist <= light_rad * light_rad) {
        float theta = atan(-rel_pos.y, -rel_pos.x) + pi;
        int ray = int(theta * light_rad);
        int ray_ycoord = ray / 128;
        int ray_xcoord = ray - ray_ycoord * 128;

        float length = texelFetch(ray_lengths, ivec2(ray_xcoord, ray_ycoord), 0).x * 256;

        if(squared_dist <= length * length) {
            float light = max(1.0 - float(squared_dist) / (light_rad * light_rad), 0.3125);
            frag_color = vec4(light, light, light, 1.0);
        } else {
            frag_color = vec4(0.3125, 0.3125, 0.3125, 1.0);
        }
    } else discard;
}
)""";*/

std::array<glm::vec2, 6> screen_vertices = {
    glm::vec2{-1, -1},
    glm::vec2{1, -1},
    glm::vec2{-1, 1},
    glm::vec2{-1, 1},
    glm::vec2{1, -1},
    glm::vec2{1, 1}
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
    glViewport(0, 0, core.screen_size.x, core.screen_size.y);

    core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
    core.reload_active_chunks = true;

    core.framebuffer.resize({core.screen_size.x, core.screen_size.y});
    core.fb_light.resize({core.screen_size.x, core.screen_size.y});
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

    Shader screen_shader;
    screen_shader.compile(vss_screen, fss_screen);

    /*Shader raycast_shader;
    raycast_shader.compile(vss_raycast, fss_raycast);

    Shader light_shader;
    light_shader.compile(vss_light, fss_light);*/

    time_t last_time = get_time();
    uint32_t delta_time;

    Buffer buffer;
    buffer.init();
    buffer.set_data(player_vertices.data(), 6 * sizeof(Vertex));
    buffer.set_attrib(0, 3, 7 * sizeof(float), 0);
    buffer.set_attrib(1, 2, 7 * sizeof(float), 3 * sizeof(float));
    buffer.set_attrib(2, 2, 7 * sizeof(float), 5 * sizeof(float));

    // framebuffer

    Buffer screen_buffer;
    screen_buffer.init();
    screen_buffer.set_data(screen_vertices.data(), sizeof(glm::vec3) * screen_vertices.size());
    screen_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);

    //
    
    glClearColor(0.0, 0.0, 0.0, 1.0);

    while(game_running) {
        glEnable(GL_DEPTH_TEST); 
        glDepthFunc(GL_ALWAYS);
        glfwPollEvents();

        // math
        time_t current_time = get_time();
        delta_time = current_time - last_time;
        last_time = current_time;

        core.math(delta_time);

        //render
        core.light_depth_buffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core.framebuffer.bind();
        GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, buffers);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //get matrix
        float w = float(screen_size.x / 2) / scale;
        float h = float(screen_size.y / 2) / scale;
        glm::mat4 pv_matrix = /*glm::perspective(100, , 0.5, 32.0);*/glm::ortho(-w, w, -h, h, -1.0f, 1.0f);
        pv_matrix *= core.camera.get_view_matrix();
        glm::mat4 flatten_mat = glm::identity<glm::mat4>();
        flatten_mat[2][1] = 1;
        pv_matrix *= flatten_mat;

        glm::mat4 pv_mat_sun = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -20.0f, 100.0f);
        pv_mat_sun *= core.sun_camera.get_view_matrix_sun();

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

                    core.framebuffer.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, &pv_matrix[0][0]);
                    glDrawArrays(GL_TRIANGLES, 0, chunk.vertex_count);

                    // render to shadowmap
                    core.light_depth_buffer.bind();
                    glUniformMatrix4fv(0, 1, GL_FALSE, &pv_mat_sun[0][0]);
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
        
        core.framebuffer.bind();
        core.player.render(player_shader, pv_matrix);

        core.light_depth_buffer.bind();
        core.player.render(player_shader, pv_mat_sun);

        // render shadowmap



        
        //glm::ivec2 position((0 - core.camera.pos.x * scale) + screen_size.x / 2, (0 - core.camera.pos.y * scale) + screen_size.y / 2);
        //glm::ivec2 mouse_pos_yinv(core.mouse_pos.x, core.screen_size.y - core.mouse_pos.y);

        // raycast
        /*core.framebuffer_light.bind();

        raycast_shader.use();
        core.framebuffer.color_tex[1].bind(0, 0);
        glUniform2iv(1, 1, &screen_size[0]);
        glUniform2iv(2, 1, &position[0]);
        glUniform1i(3, 5 * scale);
        screen_buffer.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);*/
        //

        // draw light
        /*core.fb_light.bind();
        glClearColor(0.3125, 0.3125, 0.3125, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 1.0);

        light_shader.use();
        core.framebuffer_light.color_tex[0].bind(0, 0);
        glUniform2iv(1, 1, &position[0]);
        glUniform1i(2, 5 * scale);

        screen_buffer.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);*/
        //
        

        // render framebuffer texture
        glDisable(GL_DEPTH_TEST); 
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        screen_shader.use();
        screen_buffer.bind();
        core.framebuffer.color_tex[0].bind(0, 0);
        core.framebuffer.depth_tex.bind(1, 1);
        core.light_depth_buffer.depth_tex.bind(2, 2);

        glm::mat4 inverse_matrix = glm::inverse(pv_matrix);
        glUniformMatrix4fv(3, 1, GL_FALSE, &inverse_matrix[0][0]);
        glUniformMatrix4fv(4, 1, GL_FALSE, &pv_mat_sun[0][0]);
        glUniform2iv(5, 1, &screen_size[0]);
        glUniform2f(6, -1.0f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);

        game_running = !glfwWindowShouldClose(window);
    }

    if(core.active_chunk_thread.joinable()) core.active_chunk_thread.join();
    if(core.chunk_update_thread.joinable()) core.chunk_update_thread.join();

    glfwTerminate();
    std::cout << "Successfully terminated!\n";

    return 0;
}