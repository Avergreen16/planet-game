#define USE_2D
#include "wrapper.cpp"
#include "fstream"
#include <map>
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

struct Text {
    glm::ivec2 size = {0, 0};
    Buffer text_buffer;
    GLuint color_buffer;
    std::string text = "";
    int text_size = 3;

    glm::vec2 position = glm::vec2(0, 0);

    void init_buffers() {
        glGenBuffers(1, &color_buffer);
        text_buffer.init();
    }

    void load_buffers(Font_data& font, int limit = 0x7FFFFFFF) {
        size = {0, 0};

        glm::ivec2 loc = {0, 0};
        int word_loc = 0;

        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        std::vector<Vertex> word_vertices;
        std::string word;

        int space_counter = 0;
        bool erase_space_counter = false;

        int hue = 0;

        char last_char = '\0';
        char last_word_char = '\0';

        for(char c : text) {
            if(c == '\n') {
                for(Vertex& v : word_vertices) {
                    v.pos += loc;
                    vertices.push_back(v);
                }
                last_word_char = last_char;
                word_vertices.clear();
                
                size.x = std::max(size.x, loc.x + word_loc - space_counter * !erase_space_counter * font.glyph_map[' '].stride - (font.at(last_char).stride - font.at(last_char).tex_width));
                word_loc = 0;
                loc.x = 0;
                loc.y -= font.line_spacing;
            } else {
                Glyph_data data = font.glyph_map[c];
                if(data.visible) {
                    if(!erase_space_counter) {
                        loc.x += space_counter * font.glyph_map[' '].stride;
                        erase_space_counter = true;
                    }

                    if(word_loc > 0 && (word_loc + data.tex_width) * text_size >= limit) {
                        for(Vertex& v : word_vertices) {
                            v.pos += loc;
                            vertices.push_back(v);
                        }
                        last_word_char = last_char;
                        word_vertices.clear();

                        size.x = std::max(size.x, loc.x + word_loc - (font.glyph_map[last_char].stride - font.glyph_map[last_char].tex_width));
                        word_loc = 0;
                        loc.x = 0;
                        loc.y -= font.line_spacing;
                    } else if((loc.x + word_loc + data.tex_width) * text_size >= limit) {
                        size.x = std::max(size.x, loc.x - space_counter * font.glyph_map[' '].stride - (font.at(last_word_char).stride - font.at(last_word_char).tex_width));
                        loc.x = 0;
                        loc.y -= font.line_spacing;
                    }

                    word_vertices.push_back({{word_loc, 0}, {data.tex_coord, 0}});
                    word_vertices.push_back({{word_loc + data.tex_width, 0}, {data.tex_coord + data.tex_width, 0}});
                    word_vertices.push_back({{word_loc, font.line_height}, {data.tex_coord, font.line_height}});
                    word_vertices.push_back({{word_loc, font.line_height}, {data.tex_coord, font.line_height}});
                    word_vertices.push_back({{word_loc + data.tex_width, 0}, {data.tex_coord + data.tex_width, 0}});
                    word_vertices.push_back({{word_loc + data.tex_width, font.line_height}, {data.tex_coord + data.tex_width, font.line_height}});

                    float red = (hue < 128) ? 1.0f : (hue < 256) ? (float(256 - hue) / 128.0f) : (hue < 512) ? 0.0f : (hue < 640) ? (float(hue - 512) / 128.0f) : 1.0f; 
                    float green = (hue < 128) ? (float(hue) / 128.0f) : (hue < 384) ? 1.0f : (hue < 512) ? (float(512 - hue) / 128.0f) : 0.0f;
                    float blue = (hue < 256) ? 0.0f : (hue < 384) ? (float(hue - 256) / 128.0f) : (hue < 640) ? 1.0f : (float(768 - hue) / 128.0f);

                    colors.push_back({red, green, blue, 1.0f});
                    colors.push_back({red, green, blue, 1.0f});

                    hue = (hue + 20) % 768;

                    word_loc += data.stride;

                    last_char = c;
                } else {
                    if(erase_space_counter) {
                        space_counter = 0;
                        erase_space_counter = false;

                        for(Vertex& v : word_vertices) {
                            v.pos += loc;
                            vertices.push_back(v);
                        }
                        last_word_char = last_char;
                        word_vertices.clear();
                        loc.x += word_loc;
                        word_loc = 0;
                    }
                    space_counter++;
                }
            }
        }
        for(Vertex& v : word_vertices) {
            v.pos += loc;
            vertices.push_back(v);
        }
        loc.x += word_loc;

        for(Vertex& v : vertices) {
            v.pos *= text_size;
        }

        size = {std::max(size.x, loc.x - (font.at(last_char).stride - font.at(last_char).tex_width)), -loc.y + font.line_height};
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, color_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);

        text_buffer.bind();
        text_buffer.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        text_buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
        text_buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));

        //std::cout << size.x << " " << size.y << std::endl;
    }

    void bind_buffers() {
        text_buffer.bind();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, color_buffer);
    }
};

struct Core {
    glm::ivec2 screen_size = {800, 600};
    bool game_running = true;
    bool right_shift_pressed = false;

    Font_data font;

    Text string;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    core.screen_size.x = width + 1 * (width & 1);
    core.screen_size.y = height + 1 * (height & 1);

    glViewport(0, 0, core.screen_size.x, core.screen_size.y);
    
    core.string.load_buffers(core.font, core.screen_size.x - 12);
}

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    char char_codepoint = (char)codepoint;
    if(core.right_shift_pressed && char_codepoint <= 'F' && char_codepoint >= 'A') char_codepoint += 63;
    if(core.font.glyph_map.contains(char_codepoint)) {
        core.string.text += char_codepoint;
        if(core.font.glyph_map[char_codepoint].visible) {
            core.string.load_buffers(core.font, core.screen_size.x - 12);
        }
    } else {
        std::cout << (int)char_codepoint << std::endl;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    if(key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        int string_length = core.string.text.size();
        if(string_length != 0) {
            char last_char = core.string.text[string_length - 1];
            core.string.text.pop_back();
            if(core.font.glyph_map.contains(last_char) && core.font.glyph_map[last_char].visible) {
                core.string.load_buffers(core.font, core.screen_size.x - 12);
            }
        }
    } else if(key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        core.string.text += '\n';
    } else if(key == GLFW_KEY_RIGHT_SHIFT) {
        if(action == GLFW_PRESS) core.right_shift_pressed = true;
        if(action == GLFW_RELEASE) core.right_shift_pressed = false;
    }
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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetKeyCallback(window, key_callback);

    // loading data

    std::ifstream file;
    file.open("res\\text_data.bin", std::ios::in | std::ios::binary);

    if(file.is_open()) {
        file.read((char*)&core.font.line_height, 1);
        file.read((char*)&core.font.line_spacing, 1);

        uint16_t space_glyphs;
        file.read((char*)&space_glyphs, 2);

        for(int i = 0; i < space_glyphs; i++) {
            uint8_t id;
            Glyph_data data;

            file.read((char*)&id, 1);
            file.read((char*)&data.stride, 1);

            data.visible = false;

            core.font.glyph_map.insert({id, data});
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

            core.font.glyph_map.insert({id, data});
        }

        file.close();
    } else {
        std::cout << "error" << std::endl;
    }

    // button generation

    Shader text_shader;

    text_shader.compile(get_text_from_file("res\\shaders\\text.vs").data(), get_text_from_file("res\\shaders\\text.fs").data());


    stbi_set_flip_vertically_on_load(true);
    Texture text_texture;
    text_texture.load("res\\text.png");
    text_texture.bind(0);

    core.string.init_buffers();

    std::vector<Text> texts(2);
    texts[0].text = "Open File";
    texts[0].position = {400, 500};
    texts[1].text = "New File";
    texts[1].position = {600, 300};
    for(Text& t : texts) {
        t.init_buffers();
        t.load_buffers(core.font);
    }

    while(core.game_running) {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat3 view_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(core.screen_size / 2)));
        view_matrix = glm::translate(view_matrix, glm::vec2(-core.screen_size.x / 2, -core.screen_size.y / 2));

        glm::mat3 transform_matrix = glm::translate(identity_matrix, glm::vec2(6, core.screen_size.y / 2));

        text_shader.use();
        core.string.bind_buffers();
        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, core.string.text_buffer.vertices);

        for(Text& t : texts) {
            text_shader.use();
            t.bind_buffers();

            transform_matrix = glm::translate(identity_matrix, t.position);
            glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, t.text_buffer.vertices);
        }
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);

        core.game_running = !glfwWindowShouldClose(window);
    }
}