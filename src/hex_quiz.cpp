#define USE_2D
#include "wrapper.cpp"
#include <fstream>
#include <sstream>

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

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

    void load_font_data(std::string filepath) {
        std::ifstream file;
        file.open(filepath.data(), std::ios::in | std::ios::binary);

        if(file.is_open()) {
            file.read((char*)&line_height, 1);
            file.read((char*)&line_spacing, 1);

            uint16_t space_glyphs;
            file.read((char*)&space_glyphs, 2);

            for(int i = 0; i < space_glyphs; ++i) {
                uint8_t id;
                Glyph_data data;

                file.read((char*)&id, 1);
                file.read((char*)&data.stride, 1);

                data.visible = false;

                glyph_map.insert({id, data});
            }

            uint16_t visible_glyphs;
            file.read((char*)&visible_glyphs, 2);

            for(int i = 0; i < visible_glyphs; ++i) {
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
    }
};

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

void insert_char(std::vector<Vertex>& vertices, Font_data& font, int size, Glyph_data& g, glm::ivec2 pos) {
    vertices.push_back({{pos.x, pos.y}, {g.tex_coord, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y + font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});
}

std::string to_hex_string(int num) {
    std::string return_string;
    for(int i = sizeof(num) * 2 - 1; i >= 0; --i) {
        uint8_t nibble = (num >> (i * 4)) & 0xF;
        if(nibble == 0) {
            if(return_string.size() != 0) return_string += '0';
        } else if(nibble <= 9) {
            return_string += '0' + nibble;
        } else {
            return_string += 0x80 + nibble - 0xA;
        }
    }
    if(return_string.size() == 0) return_string = "0";
    return return_string;
}

int to_hex_number(std::string string) {
    int return_int = 0;
    for(uint8_t c : string) {
        return_int <<= 4;
        if(c >= '0' && c <= '9') {
            return_int |= c - '0';
        } else if(c >= 0x80 && c <= 0x85) {
            return_int |= int(c) - 0x80 + 0xA;
        }
    }
    return return_int;
}

struct Text {
    Buffer vertex_buf;
    glm::ivec2 size;
    glm::ivec2 position;

    void init_buffers() {
        vertex_buf.init();
    }

    void load_buffers(Font_data& font, std::string text, int size) {
        int pos = 0;

        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        for(char c : text) {
            if(font.glyph_map.contains(c)) {
                Glyph_data& g = font.glyph_map[c];
                
                if(g.visible) {
                    insert_char(vertices, font, size, g, {pos, 0});
                }
                pos += g.stride * size;
            }
        }

        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        this->size = glm::ivec2{std::max(0, pos - 1 * size), font.line_height * size};
    }
};

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};

    Shader text_shader;
    Font_data font;
    Texture font_texture;

    Text prompt;
    Text input;
    Text result;

    int focus = 0x0;

    bool answered = false;
    bool correct = false;
    int correct_answer = 0;

    std::string input_text;

    void resize() {
        prompt.position = screen_size / 2 + glm::ivec2{-font.glyph_map['0'].tex_width * 2 - prompt.size.x / 2, 0};

        input.position = prompt.position + glm::ivec2{prompt.size.x + font.glyph_map[' '].stride * 2 + 2, 0};

        result.position = screen_size / 2 + glm::ivec2(-result.size.x / 2, -font.line_spacing * 2);
    }

    void new_question() {
        input_text.clear();
        input.vertex_buf.vertices = 0;

        int num1;
        int num2;
        if(focus == 0) {
            num1 = rand() % 14 + 2;
            num2 = rand() % 14 + 2;
        } else {
            if(rand() & 1) {
                num1 = rand() % 14 + 2;
                num2 = focus;
            } else {
                num1 = focus;
                num2 = rand() % 14 + 2;
            }
        }
        correct_answer = num1 * num2;

        prompt.load_buffers(font, to_hex_string(num1) + " * " + to_hex_string(num2) + " =", 2);
        answered = false;
    }

    void init() {
        text_shader.compile(get_text_from_file("res\\shaders\\text_color.vs").data(), get_text_from_file("res\\shaders\\text_color.fs").data());
        font.load_font_data("res\\text_data.bin");
        font_texture.load("res\\text.png");

        prompt.init_buffers();
        input.init_buffers();
        result.init_buffers();

        new_question();
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

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    if(codepoint >= '0' && codepoint <= '9') {
        core.input_text += codepoint;
    } else if(codepoint >= 'a' && codepoint <= 'f') {
        core.input_text += 0x80 + codepoint - 'a';
    } else if(codepoint >= 'A' && codepoint <= 'F') {
        core.input_text += 0x80 + codepoint - 'A';
    }

    core.input.load_buffers(core.font, core.input_text, 2);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    if(key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT) && core.input_text.size() != 0) {
        core.input_text.pop_back();
        core.input.load_buffers(core.font, core.input_text, 2);
    } else if(key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if(core.answered) {
            core.new_question();
        } else if(core.input_text.size() != 0) {
            int answer = to_hex_number(core.input_text);
            if(answer == core.correct_answer) {
                core.correct = true;
                core.result.load_buffers(core.font, "Correct!", 2);
                core.result.position = core.screen_size / 2 + glm::ivec2(-core.result.size.x / 2, -core.font.line_spacing * 2);
            } else {
                core.correct = false;
                core.result.load_buffers(core.font, "Incorrect... the correct answer is " + to_hex_string(core.correct_answer), 2);
                core.result.position = core.screen_size / 2 + glm::ivec2(-core.result.size.x / 2, -core.font.line_spacing * 2);
            }
            core.answered = true;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
}

void Core::game_loop() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat3 view_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2)));
    view_matrix = glm::translate(view_matrix, glm::vec2(-viewport_size / 2));

    glm::mat3 transform_matrix;

    text_shader.use();
    font_texture.bind(0);

    prompt.vertex_buf.bind();
    transform_matrix = glm::translate(identity_matrix, glm::vec2(prompt.position));

    glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
    glUniform4f(2, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, prompt.vertex_buf.vertices);

    input.vertex_buf.bind();
    transform_matrix = glm::translate(identity_matrix, glm::vec2(input.position));

    glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
    glUniform4f(2, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, input.vertex_buf.vertices);

    if(answered) {
        result.vertex_buf.bind();
        transform_matrix = glm::translate(identity_matrix, glm::vec2(result.position));

        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        if(correct) glUniform4f(2, 0.0f, 1.0f, 0.0f, 1.0f);
        else glUniform4f(2, 1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, result.vertex_buf.vertices);
    }

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
    glfwSetCharCallback(core.window, char_callback);
    glfwSetKeyCallback(core.window, key_callback);

    core.init();

    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}