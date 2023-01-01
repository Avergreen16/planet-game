#define USE_2D
#include "wrapper.cpp"
#include "fstream"
#include <map>
#include <sstream>

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

std::string get_shader_from_file(char* path) {
    std::ifstream file;
    file.open(path);

    if(file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();

    } else {
        std::cout << "failed to open file " << path << std::endl;
    }

    file.close();
}

struct Core {
    glm::ivec2 screen_size = {800, 600};
    bool game_running = true;
};

struct Glyph_data {
    bool visible;
    uint8_t stride;
    uint16_t tex_coord;
    uint8_t tex_width;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex_coords;
};

int main() {
    Core core;

    // init glfw and set version
    if(glfwInit() == GLFW_FALSE) {
        std::cout << "ERROR: GLFW failed to load.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window
    GLFWwindow* window = glfwCreateWindow(core.screen_size.x, core.screen_size.y, "This is a test.", NULL, NULL);
    if(window == NULL) {
        std::cout << "ERROR: Window creation failed.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // init glad and set viewport
    if(!gladLoadGL()) {
        std::cout << "ERROR: GLAD failed to load.\n";
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, core.screen_size.x, core.screen_size.y);

    glfwSetWindowUserPointer(window, &core);

    // loading data
    
    uint8_t line_height;
    std::map<uint8_t, Glyph_data> glyph_map;

    std::ifstream file;
    file.open("res\\text_data.bin", std::ios::in | std::ios::binary);

    if(file.is_open()) {
        file.read((char*)&line_height, 1);

        uint16_t space_glyphs;
        file.read((char*)&space_glyphs, 2);

        for(int i = 0; i < space_glyphs; i++) {
            uint8_t id;
            Glyph_data data;

            file.read((char*)&id, 1);
            file.read((char*)&data.stride, 1);

            data.visible = false;

            glyph_map.insert({id, data});
        }

        uint16_t visible_glyphs;
        file.read((char*)&visible_glyphs, 2);

        for(int i = 0; i < visible_glyphs; i++) {
            uint8_t id;
            Glyph_data data;

            file.read((char*)&id, 1);
            file.read((char*)&data.stride, 1);
            file.read((char*)&data.tex_coord, 2);
            file.read((char*)&data.tex_width, 1);

            data.visible = true;

            glyph_map.insert({id, data});
        }

        file.close();
    } else {
        std::cout << "error" << std::endl;
    }

    // text mesh generation

    
    Shader text_shader;
    text_shader.compile(get_shader_from_file("res\\shaders\\text.vs").data(), get_shader_from_file("res\\shaders\\text.fs").data());

    stbi_set_flip_vertically_on_load(true);
    Texture text_texture;
    text_texture.load("res\\text.png");
    text_texture.bind(0);

    std::string text = "Hello world!";
    text += (char)0x80;
    
    int text_size = 2;
    int loc = 0;

    std::vector<Vertex> vertex_vec;
    std::vector<glm::vec4> color_vec;

    for(char c : text) {
        Glyph_data data = glyph_map[c];

        if(data.visible) {
            vertex_vec.push_back({{loc, 0}, {data.tex_coord, 0}});
            vertex_vec.push_back({{loc + data.tex_width, 0}, {data.tex_coord + data.tex_width, 0}});
            vertex_vec.push_back({{loc, line_height}, {data.tex_coord, line_height}});
            vertex_vec.push_back({{loc, line_height}, {data.tex_coord, line_height}});
            vertex_vec.push_back({{loc + data.tex_width, 0}, {data.tex_coord + data.tex_width, 0}});
            vertex_vec.push_back({{loc + data.tex_width, line_height}, {data.tex_coord + data.tex_width, line_height}});

            color_vec.push_back({1.0f, 1.0f, 1.0f, 1.0f});
            color_vec.push_back({1.0f, 1.0f, 1.0f, 1.0f});
        }

        loc += data.stride;
    }

    for(Vertex& v : vertex_vec) {
        v.pos *= text_size;
    }

    GLuint color_buffer;
    glGenBuffers(1, &color_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, color_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, color_vec.size() * sizeof(glm::vec4), color_vec.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, color_buffer);

    Buffer text_buffer;
    text_buffer.init();
    text_buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex));
    text_buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
    text_buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));

    //

    glm::mat3 matrix = glm::translate(matrix, -glm::vec2(core.screen_size / 2));
    matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(core.screen_size / 2)));

    while(core.game_running) {
        glfwPollEvents();

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        text_shader.use();
        text_buffer.bind();
        glUniformMatrix3fv(0, 1, false, &matrix[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, text_buffer.vertices);

        glfwSwapBuffers(window);

        core.game_running = !glfwWindowShouldClose(window);
    }
}