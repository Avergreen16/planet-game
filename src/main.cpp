#include "global.cpp"
#include "core.cpp"
#include "chunk.cpp"
#include <unordered_map>
#include <chrono>
#include <iostream>

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

const char* fragment_shader_source = R"""(
#version 460 core
out vec4 frag_color;

in vec2 tex_coord;
in vec2 limit;
uniform sampler2D texture_input;

void main() {
    frag_color = texelFetch(texture_input, ivec2(min(tex_coord.x, limit.x), min(tex_coord.y, limit.y)), 0);
} 
)""";

std::unordered_map<GLuint, bool> keymap = {
    {GLFW_KEY_W, false},
    {GLFW_KEY_A, false},
    {GLFW_KEY_S, false},
    {GLFW_KEY_D, false}
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        if(keymap.contains(key)) {
            keymap[key] = true;
        }
    } else if(action == GLFW_RELEASE) {
        if(keymap.contains(key)) {
            keymap[key] = false;
        }
    }
}

int main() {
    stbi_set_flip_vertically_on_load(true);
    int width = 800, height = 600, scale = 48;
    // init glfw
    if(glfwInit() == GLFW_FALSE) {
        std::cout << "ERROR: GLFW failed to load.\n";
        return -1;
    }

    // set version and create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, "This is a test.", NULL, NULL);
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

    // load chunk
    siv::PerlinNoise noise(0x1000);
    Chunk* chunk = Chunk::generate_chunk(noise, {0, 0});
    chunk->generate_mesh();
    chunk->buffer.set_attrib(0, 3, 7 * sizeof(float), 0);
    chunk->buffer.set_attrib(1, 2, 7 * sizeof(float), 3 * sizeof(float));
    chunk->buffer.set_attrib(2, 2, 7 * sizeof(float), 5 * sizeof(float));

    // load texture
    Texture texture;
    texture.load("res\\tiles.png");

    // create shader
    Shader shader;
    shader.compile(vertex_shader_source, fragment_shader_source);

    // create camera
    Camera camera({0.0, 0.0});

    time_t last_time = get_time();
    unsigned int delta_time;

    while(!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glfwPollEvents();

        // math
        time_t current_time = get_time();
        delta_time = current_time - last_time;
        last_time = current_time;

        double move_dist = 0.000000005 * delta_time;
        if(keymap[GLFW_KEY_W]) {
            camera.pos.y += move_dist;
        }
        if(keymap[GLFW_KEY_A]) {
            camera.pos.x -= move_dist;
        }
        if(keymap[GLFW_KEY_S]) {
            camera.pos.y -= move_dist;
        }
        if(keymap[GLFW_KEY_D]) {
            camera.pos.x += move_dist;
        }
        //std::cout << camera.pos.x << " " << camera.pos.y << "\n";

        //render
        glClearColor(0.2, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        //get matrix
        glm::mat4 pv_matrix = glm::ortho(-float(width / 2) / scale, float(width / 2) / scale, -float(height / 2) / scale, float(height / 2) / scale, 0.0f, 16.0f);
        pv_matrix *= camera.get_view_matrix();

        // draw
        shader.use();
        texture.bind();
        chunk->buffer.bind();

        glUniformMatrix4fv(0, 1, GL_FALSE, &pv_matrix[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, chunk->vertex_count);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    std::cout << "Successfuly terminated!\n";

    // free memory
    delete(chunk);

    return 0;
}