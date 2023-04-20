#define USE_2D
#include "wrapper.cpp"
#include <fstream>
#include <sstream>
#include <chrono>

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex_coords;
};

struct glyph_data {
    bool visible;
    uint16_t tex_coord;
    uint8_t tex_width;
    uint8_t advance1;
    uint8_t advance2;
};

struct Font_data {
    uint8_t line_height;
    uint8_t line_spacing;
    glyph_data empty_data = {false, 0, 0, 0};
    std::map<char, glyph_data> glyph_map;

    glyph_data& at(char key) {
        if(glyph_map.contains(key)) return glyph_map[key];
        return empty_data;
    }

    void load_font_data(std::string filepath) {
        std::ifstream file;
        file.open(filepath, std::ios::in | std::ios::binary);

        if(file.is_open()) {
            file.read((char*)&line_height, 1);
            file.read((char*)&line_spacing, 1);

            uint16_t space_glyphs;
            file.read((char*)&space_glyphs, 2);

            for(int i = 0; i < space_glyphs; ++i) {
                uint8_t id;
                glyph_data data;

                file.read((char*)&id, 1);
                file.read((char*)&data.advance1, 1);
                data.advance2 = 0;
                data.tex_width = 0;
                data.tex_coord = 0;

                data.visible = false;

                glyph_map.insert({id, data});
            }

            uint16_t visible_glyphs;
            file.read((char*)&visible_glyphs, 2);

            for(int i = 0; i < visible_glyphs; ++i) {
                uint8_t id;
                glyph_data data;

                file.read((char*)&id, 1);
                file.read((char*)&data.tex_coord, 2);
                file.read((char*)&data.tex_width, 1);
                file.read((char*)&data.advance1, 1);
                file.read((char*)&data.advance2, 1);

                data.visible = true;

                glyph_map.insert({id, data});
            }

            file.close();
        } else {
            std::cout << "error" << std::endl;
        }
    }
};

void insert_char(std::vector<Vertex>& vertices, Font_data& font, int size, glyph_data& g, glm::ivec2 pos) {
    vertices.push_back({{pos.x, pos.y}, {g.tex_coord, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y + font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});
}

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

struct Text {
    Buffer vertex_buf;
    glm::ivec2 size;
    glm::ivec2 position;

    void init_buffers() {
        vertex_buf.init();
    }

    void load_buffers(Font_data& font, std::string text, int text_size) {
        int pos = 0;

        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        for(char c : text) {
            if(font.glyph_map.contains(c)) {
                glyph_data& g = font.glyph_map[c];

                pos += g.advance1 * text_size;
                
                if(g.visible) {
                    insert_char(vertices, font, text_size, g, {pos, 0});
                }
                pos += (g.tex_width + g.advance2) * text_size;
            }
        }

        vertex_buf.bind();
        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        size = glm::ivec2{std::max(0, pos - 1 * text_size), font.line_height * text_size};
    }
};

std::string to_hex_string(uint16_t num) {
    std::string return_string;
    for(int i = sizeof(num) * 2 - 1; i >= 0; --i) {
        uint8_t nibble = (num >> (i * 4)) & 0xF;
        if(nibble == 0) {
            return_string += '0';
        } else if(nibble <= 9) {
            return_string += '0' + nibble;
        } else {
            return_string += 0x80 + nibble - 0xA;
        }
    }
    if(return_string.size() == 0) return_string = "0";
    return return_string;
}

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {256, 256};
    glm::ivec2 viewport_size = {256, 256};

    Shader text_shader;
    Font_data font;
    Texture font_texture;

    Text hex_time;
    Text dec_time;

    int h_units = -1;
    int sec = -1;

    void resize() {
        dec_time.position = screen_size / 2 + glm::ivec2{-dec_time.size.x / 2, font.line_height * 1.5};
        hex_time.position = screen_size / 2 + glm::ivec2{-hex_time.size.x / 2, -font.line_height * 4.5};
    }

    void get_time() {
        std::chrono::milliseconds mil = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        uint64_t ms_day = ((mil.count() - 18000000) % 86400000);
        int milliseconds = ms_day % 1000;
        int seconds = (ms_day - milliseconds) % 60000;
        uint16_t hex_units = 0x10000 * (double(ms_day) / 86400000);

        if(seconds != sec) {
            sec = seconds;

            int minutes = (ms_day - seconds - milliseconds) % 3600000;
            int hours = (ms_day / 3600000) % 12;
            std::string h;
            std::string m;
            std::string s;
            std::string ms;

            if(hours == 0) hours = 12;
            if(hours < 10) h = "0";
            h += std::to_string(hours);
            
            minutes /= 60000;
            if(minutes < 10) m = "0";
            m += std::to_string(minutes);

            seconds /= 1000;
            if(seconds < 10) s = "0";
            s += std::to_string(seconds);

            std::string d_time = h + ":" + m + ":" + s;

            dec_time.load_buffers(font, d_time, 3);
        }

        if(hex_units != h_units) {
            h_units = hex_units;
            std::string h_time = to_hex_string(hex_units);
            h_time.insert(h_time.begin() + 2, ':');

            hex_time.load_buffers(font, h_time, 3);
        }
    }

    void init() {
        text_shader.compile(get_text_from_file("res\\shaders\\text_color.vs").data(), get_text_from_file("res\\shaders\\text_color.fs").data());
        font.load_font_data("res\\text_data.bin");
        font_texture.load("res\\text.png");

        hex_time.init_buffers();
        dec_time.init_buffers();
        
        get_time();
        resize();
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

    core.resize();

    core.game_loop();
}

void Core::game_loop() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    get_time();

    glm::mat3 view_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2)));
    view_matrix = glm::translate(view_matrix, glm::vec2(-viewport_size / 2));

    glm::mat3 transform_matrix;

    text_shader.use();
    font_texture.bind(0);

    dec_time.vertex_buf.bind();
    transform_matrix = glm::translate(identity_matrix, glm::vec2(dec_time.position));

    glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
    glUniform4f(2, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, dec_time.vertex_buf.vertices);

    hex_time.vertex_buf.bind();
    transform_matrix = glm::translate(identity_matrix, glm::vec2(hex_time.position));

    glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
    glUniform4f(2, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, hex_time.vertex_buf.vertices);

    glfwSwapBuffers(window);
}

int main() {
    stbi_set_flip_vertically_on_load(true);

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
    core.window = glfwCreateWindow(core.screen_size.x, core.screen_size.y, "Hex Time", NULL, NULL);
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

    core.init();

    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}