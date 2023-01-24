#define USE_2D
#include "wrapper.cpp"
#include <fstream>
#include <sstream>

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

std::string get_text_from_file(char* path) {
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

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex_coords;
};

struct Glyph_data {
    bool visible;
    uint8_t stride;
    uint16_t tex_coord;
    uint8_t tex_width;
};

struct Font_data {
    uint8_t line_height;
    uint8_t line_spacing;
    Glyph_data empty_data = {false, 0, 0, 0};
    std::map<char, Glyph_data> glyph_map;

    Glyph_data& at(char key) {
        if(glyph_map.contains(key)) return glyph_map[key];
        return empty_data;
    }
};

struct AABB {
    glm::ivec2 size;
    glm::ivec2 position;

    bool contains(glm::ivec2 point) {
        return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y && point.y <= position.y + size.y;
    }

    bool contains(AABB box) {
        return box.position.x + box.size.x >= position.x && box.position.x <= position.x + size.x && box.position.y + box.size.y >= position.y && box.position.y <= position.y + size.y;
    }
};

struct Text_row {
    std::vector<uint8_t> bytes;
    Buffer vertex_buf;
    Storage_buffer color_buf;

    void init_buffers() {
        vertex_buf.init();
        color_buf.init();
    }

    void load_buffers(Font_data& font, int size) {
        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        int pos = 0;
        for(uint8_t byte : bytes) {
            uint8_t a = byte >> 4;
            uint8_t b = byte & 0xF;
            Glyph_data ga = font.glyph_map[(a < 0xA) ? 0x30 + a : 0x76 + a];
            Glyph_data gb = font.glyph_map[(b < 0xA) ? 0x30 + b : 0x76 + b];

            vertices.push_back({{pos, 0}, {ga.tex_coord, 0}});
            vertices.push_back({{pos + ga.tex_width * size, 0}, {ga.tex_coord + ga.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {ga.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {ga.tex_coord, font.line_height}});
            vertices.push_back({{pos + ga.tex_width * size, 0}, {ga.tex_coord + ga.tex_width, 0}});
            vertices.push_back({{pos + ga.tex_width * size, font.line_height * size}, {ga.tex_coord + ga.tex_width, font.line_height}});

            pos += (ga.tex_width + 1) * size;

            vertices.push_back({{pos, 0}, {gb.tex_coord, 0}});
            vertices.push_back({{pos + gb.tex_width * size, 0}, {gb.tex_coord + gb.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {gb.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {gb.tex_coord, font.line_height}});
            vertices.push_back({{pos + gb.tex_width * size, 0}, {gb.tex_coord + gb.tex_width, 0}});
            vertices.push_back({{pos + gb.tex_width * size, font.line_height * size}, {gb.tex_coord + gb.tex_width, font.line_height}});

            pos += (gb.tex_width + 5) * size;
        }

        vertex_buf.set_data(&vertices, vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, sizeof(float) * 2);

        color_buf.set_data(&colors, colors.size() * sizeof(glm::vec4));
    }

    void load_buffers(Font_data& font, int size, std::vector<uint8_t>&& new_bytes) {
        bytes = new_bytes;

        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        int pos = 0;
        for(uint8_t byte : bytes) {
            uint8_t a = byte >> 4;
            uint8_t b = byte & 0xF;
            Glyph_data ga = font.glyph_map[(a < 0xA) ? 0x30 + a : 0x76 + a];
            Glyph_data gb = font.glyph_map[(b < 0xA) ? 0x30 + b : 0x76 + b];

            vertices.push_back({{pos, 0}, {ga.tex_coord, 0}});
            vertices.push_back({{pos + ga.tex_width * size, 0}, {ga.tex_coord + ga.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {ga.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {ga.tex_coord, font.line_height}});
            vertices.push_back({{pos + ga.tex_width * size, 0}, {ga.tex_coord + ga.tex_width, 0}});
            vertices.push_back({{pos + ga.tex_width * size, font.line_height * size}, {ga.tex_coord + ga.tex_width, font.line_height}});

            pos += (ga.tex_width + 1) * size;

            vertices.push_back({{pos, 0}, {gb.tex_coord, 0}});
            vertices.push_back({{pos + gb.tex_width * size, 0}, {gb.tex_coord + gb.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {gb.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {gb.tex_coord, font.line_height}});
            vertices.push_back({{pos + gb.tex_width * size, 0}, {gb.tex_coord + gb.tex_width, 0}});
            vertices.push_back({{pos + gb.tex_width * size, font.line_height * size}, {gb.tex_coord + gb.tex_width, font.line_height}});

            pos += (gb.tex_width + 5) * size;

            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
        }

        vertex_buf.set_data(&vertices, vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        color_buf.set_data(&colors, colors.size() * sizeof(glm::vec4));
    }
};

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};

    std::vector<Shader> shaders;
    std::vector<Texture> textures;

    Font_data font;
    int text_size = 2;
    std::map<uint32_t, Text_row> byte_rows;

    void load_font() {
        std::ifstream file;
        file.open("res\\text_data.bin", std::ios::in | std::ios::binary);

        if(file.is_open()) {
            file.read((char*)&font.line_height, 1);
            file.read((char*)&font.line_spacing, 1);

            uint16_t space_glyphs;
            file.read((char*)&space_glyphs, 2);

            for(int i = 0; i < space_glyphs; i++) {
                uint8_t id;
                Glyph_data data;

                file.read((char*)&id, 1);
                file.read((char*)&data.stride, 1);

                data.visible = false;

                font.glyph_map.insert({id, data});
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

                font.glyph_map.insert({id, data});
            }

            file.close();
        } else {
            std::cout << "error" << std::endl;
        }
    }

    void load_byte_rows(std::vector<uint8_t> bytes) {
        int pos = 0;
        for(int i = 0; ; i++) {
            byte_rows.insert({i, Text_row()});
            byte_rows[i].init_buffers();

            if(pos + 8 < bytes.size()) {
                byte_rows[i].load_buffers(font, text_size, std::vector<uint8_t>(bytes.begin() + pos, bytes.begin() + pos + 8));
                pos += 8;
            } else {
                byte_rows[i].load_buffers(font, text_size, std::vector<uint8_t>(bytes.begin() + pos, bytes.end()));
                break;
            }
        }
    }

    void game_loop();
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    core.screen_size.x = width;
    core.screen_size.y = height;
    core.viewport_size.x = width + 1 * (width & 1);
    core.viewport_size.y = height + 1 * (height & 1);

    glViewport(0, 0, core.viewport_size.x, core.viewport_size.y);

    core.game_loop();
}

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
}

void Core::game_loop() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat3 view_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2)));
    view_matrix = glm::translate(view_matrix, glm::vec2(-viewport_size.x / 2, -viewport_size.y / 2));

    glm::mat3 transform_matrix = glm::translate(identity_matrix, glm::vec2(6, screen_size.y - 10 * text_size));

    shaders[0].use();
    textures[0].bind(0);
    //for(auto& [i, t] : byte_rows) {
    Text_row& t = byte_rows[0];
        t.vertex_buf.bind();
        t.color_buf.bind(0);

        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &identity_matrix[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, t.vertex_buf.vertices);

        //transform_matrix = glm::translate(transform_matrix, glm::vec2(0, -14 * text_size));
    //}

    glfwSwapBuffers(window);
}

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
    core.window = glfwCreateWindow(core.screen_size.x, core.screen_size.y, "This is a test.", NULL, NULL);
    if(core.window == NULL) {
        std::cout << "ERROR: Window creation failed.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(core.window);

    // init glad and set viewport
    if(!gladLoadGL()) {
        std::cout << "ERROR: GLAD failed to load.\n";
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, core.screen_size.x, core.screen_size.y);

    glfwSetWindowUserPointer(core.window, &core);

    glfwSetFramebufferSizeCallback(core.window, framebuffer_size_callback);
    //glfwSetCharCallback(core.window, char_callback);
    //glfwSetKeyCallback(core.window, key_callback);
    //glfwSetCursorPosCallback(core.window, cursor_pos_callback);

    core.shaders = std::vector<Shader>(1);
    core.textures = std::vector<Texture>(1);
    
    core.shaders[0].compile(get_text_from_file("res\\shaders\\text.vs").data(), get_text_from_file("res\\shaders\\text.fs").data());
    core.textures[0].load("res\\text.png");

    std::vector<uint8_t> bytes;
    for(int i = 0; i < 45; i++) {
        bytes.push_back(rand() % 256);
    }

    core.load_font();
    core.load_byte_rows(bytes);

    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}