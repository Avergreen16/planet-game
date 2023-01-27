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
    Buffer vertex_buf;
    Storage_buffer color_buf;

    void init_buffers() {
        vertex_buf.init();
        color_buf.init();
    }

    void clear_buffers() {
        vertex_buf.delete_buffers();
        color_buf.delete_buffer();
    }

    void load_buffers(Font_data& font, int size, uint32_t line_num, std::vector<uint8_t>& bytes, uint8_t ten) {
        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        int pos = 0;
        for(int i = 7; i >= 0; i--) {
            uint8_t n = (line_num >> (i * 4)) & 0xF;
            Glyph_data& g = font.glyph_map[(n < 0xA) ? 0x30 + n : ten - 10 + n];

            vertices.push_back({{pos, 0}, {g.tex_coord, 0}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos + g.tex_width * size, font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});

            pos += (g.tex_width + 1) * size;
            
            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
        }

        pos += 12 * size;

        int bytes_num = std::min(16u, (unsigned int)bytes.size() - line_num);

        for(int i = line_num; i < line_num + bytes_num; i++) {
            uint8_t byte = bytes[i];
            uint8_t a = byte >> 4;
            uint8_t b = byte & 0xF;
            Glyph_data& ga = font.glyph_map[(a < 0xA) ? 0x30 + a : ten - 10 + a];
            Glyph_data& gb = font.glyph_map[(b < 0xA) ? 0x30 + b : ten - 10 + b];

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

            pos += (gb.tex_width + 6) * size;

            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
        }

        if(bytes_num < 16) {
            Glyph_data& g = font.glyph_map['+'];

            pos += ((font.glyph_map['0'].tex_width * 2 + 1) - g.tex_width) / 2 * size;

            vertices.push_back({{pos, 0}, {g.tex_coord, 0}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos + g.tex_width * size, font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});

            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
        }

        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        color_buf.set_data(colors.data(), colors.size() * sizeof(glm::vec4));
    }

    void load_buffers(Font_data& font, int size, uint32_t line_num, std::vector<uint8_t>& bytes, uint8_t ten, int missing_index, int show) {
        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        int pos = 0;
        for(int i = 7; i >= 0; i--) {
            uint8_t n = (line_num >> (i * 4)) & 0xF;
            Glyph_data& g = font.glyph_map[(n < 0xA) ? 0x30 + n : ten - 10 + n];

            vertices.push_back({{pos, 0}, {g.tex_coord, 0}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos + g.tex_width * size, font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});

            pos += (g.tex_width + 1) * size;
            
            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
        }

        pos += 12 * size;

        int bytes_num = std::min(16u, (unsigned int)bytes.size() - line_num);

        int i;
        for(i = line_num; i < line_num + bytes_num; i++) {
            uint8_t byte = bytes[i];
            if(i - line_num == missing_index) {
                if(show == 1) {
                    uint8_t a = bytes[i] >> 4;
                    Glyph_data& ga = font.glyph_map[(a < 0xA) ? 0x30 + a : ten - 10 + a];

                    vertices.push_back({{pos, 0}, {ga.tex_coord, 0}});
                    vertices.push_back({{pos + ga.tex_width * size, 0}, {ga.tex_coord + ga.tex_width, 0}});
                    vertices.push_back({{pos, font.line_height * size}, {ga.tex_coord, font.line_height}});
                    vertices.push_back({{pos, font.line_height * size}, {ga.tex_coord, font.line_height}});
                    vertices.push_back({{pos + ga.tex_width * size, 0}, {ga.tex_coord + ga.tex_width, 0}});
                    vertices.push_back({{pos + ga.tex_width * size, font.line_height * size}, {ga.tex_coord + ga.tex_width, font.line_height}});

                    colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
                }
                pos += (font.glyph_map['0'].tex_width * 2 + 7) * size;
            } else {
                uint8_t byte = bytes[i];

                uint8_t a = byte >> 4;
                uint8_t b = byte & 0xF;
                Glyph_data& ga = font.glyph_map[(a < 0xA) ? 0x30 + a : ten - 10 + a];
                Glyph_data& gb = font.glyph_map[(b < 0xA) ? 0x30 + b : ten - 10 + b];

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

                pos += (gb.tex_width + 6) * size;

                colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
                colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
            }
        }

        if(bytes_num < 16 && missing_index != i) {
            Glyph_data& g = font.glyph_map['+'];

            pos += ((font.glyph_map['0'].tex_width * 2 + 1) - g.tex_width) / 2 * size;

            vertices.push_back({{pos, 0}, {g.tex_coord, 0}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos, font.line_height * size}, {g.tex_coord, font.line_height}});
            vertices.push_back({{pos + g.tex_width * size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertices.push_back({{pos + g.tex_width * size, font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});

            colors.push_back({0.0f, 1.0f, 0.0f, 1.0f});
        }

        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        color_buf.set_data(colors.data(), colors.size() * sizeof(glm::vec4));
    }
};

struct Scrollbar {
    int length;
    AABB bar;
    double position = 0.0;
    bool clicked_on = false;

    int max;

    Scrollbar(double pos) {
        position = pos;
    }

    void update_pos(int new_pos, bool inverse) {
        if(inverse) {
            position = 1.0 - double(new_pos) / max;
        } else {
            position = double(new_pos) / max;
        }

        position = std::clamp(position, 0.0, 1.0);
    }

    void set_val(int& val, bool inverse) {
        if(inverse) {
            val = (1.0 - position) * max;
        } else {
            val = position * max;
        }
    }
};

struct Core {
    std::map<GLuint, bool> mouse_button_map = {
        {GLFW_MOUSE_BUTTON_LEFT, false}
    };

    uint8_t ten = 0x80;

    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};
    glm::ivec2 mouse_pos = {0, 0};

    std::vector<Shader> shaders;
    std::vector<Texture> textures;
    std::vector<Buffer> buffers;

    Font_data font;
    int text_size = 2;
    
    Scrollbar scrollbar = Scrollbar(1.0);

    std::vector<uint8_t> bytes;
    std::map<uint32_t, Text_row> byte_rows;
    int start_loc = 0;
    int end_loc = 0;
    glm::ivec2 top_left_num_loc = {0, 0};
    int rows_shown = 0;
    int rows_total = 0;

    glm::ivec2 hovered_num = {0, 0};
    glm::ivec2 selected_num = {0, -1};
    int input_status = 2;

    // numbers at the top
    Buffer num_buffer;
    Storage_buffer num_color_buffer;

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

    void load_assets() {
        shaders = std::vector<Shader>(2);
        textures = std::vector<Texture>(1);
        buffers = std::vector<Buffer>(1);
        
        shaders[0].compile(get_text_from_file("res\\shaders\\text.vs").data(), get_text_from_file("res\\shaders\\text.fs").data());
        shaders[1].compile(get_text_from_file("res\\shaders\\select.vs").data(), get_text_from_file("res\\shaders\\select.fs").data());

        textures[0].load("res\\text.png");
        load_font();

        std::vector<glm::vec2> vertex_vec;

        vertex_vec.push_back({0, 0});
        vertex_vec.push_back({1, 0});
        vertex_vec.push_back({0, 1});
        vertex_vec.push_back({0, 1});
        vertex_vec.push_back({1, 0});
        vertex_vec.push_back({1, 1});

        buffers[0].init();
        buffers[0].set_data(vertex_vec.data(), vertex_vec.size(), sizeof(glm::vec2));
        buffers[0].set_attrib(0, 2, sizeof(float) * 2, 0);
    }

    void load_byte_rows() {
        std::vector<Vertex> vertex_vec;
        std::vector<glm::vec4> color_vec;
        
        int pos = 0;
        for(int n = 0; n < 0x10; n++) {
            Glyph_data& g = font.glyph_map[(n < 0xA) ? 0x30 + n : ten - 10 + n];

            vertex_vec.push_back({{pos, 0}, {g.tex_coord, 0}});
            vertex_vec.push_back({{pos + g.tex_width * text_size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertex_vec.push_back({{pos, font.line_height * text_size}, {g.tex_coord, font.line_height}});
            vertex_vec.push_back({{pos, font.line_height * text_size}, {g.tex_coord, font.line_height}});
            vertex_vec.push_back({{pos + g.tex_width * text_size, 0}, {g.tex_coord + g.tex_width, 0}});
            vertex_vec.push_back({{pos + g.tex_width * text_size, font.line_height * text_size}, {g.tex_coord + g.tex_width, font.line_height}});

            color_vec.push_back({0.0f, 1.0f, 0.0f, 1.0f});

            pos += 17 * text_size;
        }

        num_buffer.init();
        num_color_buffer.init();

        num_buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex));
        num_buffer.set_attrib(0, 2, sizeof(float) * 4, 0);
        num_buffer.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        num_color_buffer.set_data(color_vec.data(), color_vec.size() * sizeof(glm::vec4));
    }

    void update_dimensions() {
        top_left_num_loc = {20 + 57 * text_size, screen_size.y - 31 * text_size - 56};
        rows_total = bytes.size() / 16 + 1;
        rows_shown = top_left_num_loc.y / (15 * text_size) + 1;
        scrollbar.max = std::max(rows_total - rows_shown, 0);
        end_loc = std::min(start_loc + rows_shown, (int)rows_total);

        scrollbar.length = screen_size.y - 56;
        int bar_size = std::max(double(rows_shown) / rows_total * scrollbar.length, 20.0);
        scrollbar.bar = {{10, bar_size}, {0, scrollbar.length - (1.0 - scrollbar.position) * (scrollbar.length - bar_size) - bar_size}};
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

    if(core.selected_num.y != -1) {
        if(codepoint >= '0' && codepoint <= '9') {
            Text_row& row = core.byte_rows[core.selected_num.y];
            if(core.selected_num.y * 16 + core.selected_num.x == core.bytes.size()) {
                core.bytes.push_back(0x0);
                if(core.bytes.size() % 16 == 0) {
                    core.scrollbar.max += 1;
                    core.scrollbar.position = 1.0 - double(core.start_loc) / core.scrollbar.max;
                }
            }
            uint8_t& byte = core.bytes[core.selected_num.y * 16 + core.selected_num.x];
            if(core.input_status == 0 || core.input_status == -1) {
                byte = (codepoint - '0') << 4;
                row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten, core.selected_num.x, 1);
                core.input_status = 1;
            } else if(core.input_status == 1) {
                byte |= codepoint - '0';
                row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                core.input_status = 0;

                ++core.selected_num.x;
                if(core.selected_num.x > 15) {
                    core.selected_num.x = 0;
                    ++core.selected_num.y;
                }
            }

            if(core.selected_num.y < core.start_loc) {
                core.start_loc = core.selected_num.y;
                core.scrollbar.update_pos(core.start_loc, true);
            } else if(core.selected_num.y >= core.end_loc) {
                core.start_loc = core.selected_num.y - core.rows_shown + 1;
                core.scrollbar.update_pos(core.start_loc, true);
            }
        } else if(codepoint >= 'a' && codepoint <= 'f') {
            Text_row& row = core.byte_rows[core.selected_num.y];
            if(core.selected_num.y * 16 + core.selected_num.x == core.bytes.size()) {
                core.bytes.push_back(0x0);
                if(core.bytes.size() % 16 == 0) {
                    core.scrollbar.max += 1;
                    core.scrollbar.position = 1.0 - double(core.start_loc) / core.scrollbar.max;
                }
            }
            uint8_t& byte = core.bytes[core.selected_num.y * 16 + core.selected_num.x];
            if(core.input_status == 0 || core.input_status == -1) {
                byte = (codepoint - 'a' + 0xA) << 4;
                row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten, core.selected_num.x, 1);
                core.input_status = 1;
            } else if(core.input_status == 1) {
                byte |= codepoint - 'a' + 0xA;
                row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                core.input_status = 0;
                
                ++core.selected_num.x;
                if(core.selected_num.x > 15) {
                    core.selected_num.x = 0;
                    ++core.selected_num.y;
                }
            }

            if(core.selected_num.y < core.start_loc) {
                core.start_loc = core.selected_num.y;
                core.scrollbar.update_pos(core.start_loc, true);
            } else if(core.selected_num.y >= core.end_loc) {
                core.start_loc = core.selected_num.y - core.rows_shown + 1;
                core.scrollbar.update_pos(core.start_loc, true);
            }
        } else if(codepoint >= 'A' && codepoint <= 'F') {
            Text_row& row = core.byte_rows[core.selected_num.y];
            if(core.selected_num.y * 16 + core.selected_num.x == core.bytes.size()) {
                core.bytes.push_back(0x0);
                if(core.bytes.size() % 16 == 0) {
                    core.scrollbar.max += 1;
                    core.scrollbar.position = 1.0 - double(core.start_loc) / core.scrollbar.max;
                }
            }
            uint8_t& byte = core.bytes[core.selected_num.y * 16 + core.selected_num.x];
            if(core.input_status == 0 || core.input_status == -1) {
                byte = (codepoint - 'A' + 0xA) << 4;
                row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten, core.selected_num.x, 1);
                core.input_status = 1;

            } else if(core.input_status == 1) {
                byte |= codepoint - 'A' + 0xA;
                row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                core.input_status = 0;

                ++core.selected_num.x;
                if(core.selected_num.x > 15) {
                    core.selected_num.x = 0;
                    ++core.selected_num.y;
                }
            }

            if(core.selected_num.y < core.start_loc) {
                core.start_loc = core.selected_num.y;
                core.scrollbar.update_pos(core.start_loc, true);
            } else if(core.selected_num.y >= core.end_loc) {
                core.start_loc = core.selected_num.y - core.rows_shown + 1;
                core.scrollbar.update_pos(core.start_loc, true);
            }
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    if(key == GLFW_KEY_BACKSPACE) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y * 16 + core.selected_num.x == core.bytes.size()) {
                core.bytes.pop_back();

                if(core.selected_num.x == 0) {
                    if(core.selected_num.y == 0) {
                        core.selected_num.y = -1;
                    } else {
                        core.selected_num.x = 15;
                        --core.selected_num.y;
                    }
                } else {
                    --core.selected_num.x;
                }

                core.byte_rows[core.selected_num.y].load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);

                if(core.start_loc > core.selected_num.y) {
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(core.start_loc, true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = core.selected_num.y - core.rows_shown + 1;
                    core.scrollbar.update_pos(core.start_loc, true);
                }
            } else if(core.selected_num.y != -1) {
                Text_row& row = core.byte_rows[core.selected_num.y];
                int loc = core.selected_num.y * 16 + core.selected_num.x;
                if(core.input_status == 1 || core.input_status == -1) {
                    core.bytes[loc] = 0;
                    row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten, core.selected_num.x, 0);
                    core.input_status = 0;
                } else if(core.input_status == 0) {
                    core.bytes.erase(core.bytes.begin() + loc);
                    for(auto& [i, t] : core.byte_rows) {
                        if(i >= core.selected_num.y) {
                            if(i >= core.end_loc) {
                                core.byte_rows.erase(i);
                            } else {
                                t.load_buffers(core.font, core.text_size, i * 0x10, core.bytes, core.ten);
                            }
                        }
                    }

                    if(core.selected_num.x == 0) {
                        if(core.selected_num.y == 0) {
                            core.selected_num.y = -1;
                        } else {
                            core.selected_num.x = 15;
                            --core.selected_num.y;
                        }
                    } else {
                        --core.selected_num.x;
                    }

                    core.input_status = -1;
                }

                if(core.start_loc > core.selected_num.y) {
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(core.start_loc, true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = core.selected_num.y - core.rows_shown + 1;
                    core.scrollbar.update_pos(core.start_loc, true);
                }
            }
        }
    } else if(key == GLFW_KEY_UP) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                    core.input_status = -1;
                }

                core.selected_num.y = std::max(core.selected_num.y - 1, 0);
                if(core.start_loc > core.selected_num.y) {
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(core.start_loc, true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = core.selected_num.y - core.rows_shown + 1;
                    core.scrollbar.update_pos(core.start_loc, true);
                }
            }
        }
    } else if(key == GLFW_KEY_DOWN) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                    core.input_status = -1;
                }

                core.selected_num.y = std::min(core.selected_num.y + 1, core.rows_total - 1);
                if(core.selected_num.y * 16 + core.selected_num.x > core.bytes.size()) {
                    core.selected_num.x = core.bytes.size() % 16;
                }

                if(core.start_loc > core.selected_num.y) {
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(core.start_loc, true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = core.selected_num.y - core.rows_shown + 1;
                    core.scrollbar.update_pos(core.start_loc, true);
                }
            }
        }
    } else if(key == GLFW_KEY_RIGHT) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                    core.input_status = -1;
                    std::cout << "check ";
                }

                if(core.selected_num.y != core.rows_total - 1 || core.selected_num.x + 1 <= core.bytes.size() % 16) {
                    ++core.selected_num.x;
                }

                if(core.selected_num.x > 15) {
                    core.selected_num.x = 0;
                    ++core.selected_num.y;
                }
                
                if(core.start_loc > core.selected_num.y) {
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(core.start_loc, true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = core.selected_num.y - core.rows_shown + 1;
                    core.scrollbar.update_pos(core.start_loc, true);
                }
            }
        }
    } else if(key == GLFW_KEY_LEFT) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                    core.input_status = -1;
                }

                --core.selected_num.x;
                if(core.selected_num.x < 0) {
                    if(core.selected_num.y == 0) {
                        core.selected_num.x = 0;
                    } else {
                        core.selected_num.x = 15;
                        --core.selected_num.y;
                    }
                }
                
                if(core.start_loc > core.selected_num.y) {
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(core.start_loc, true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = core.selected_num.y - core.rows_shown + 1;
                    core.scrollbar.update_pos(core.start_loc, true);
                }
            }
        }
    }
    
    
    
    /* else if(key == GLFW_KEY_ENTER) {
        if(core.selected_num.y != -1) {
            Text_row& row = core.byte_rows[core.selected_num.y];
            row.load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes);
            core.input_status = 2;
            core.selected_num.y = -1;
        }
    }*/
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    glm::ivec2 prev_pos = core.mouse_pos;
    core.mouse_pos = {xpos, core.screen_size.y - ceil(ypos) - 1};

    glm::ivec2 hovered_num = {floor(double(core.mouse_pos.x - core.top_left_num_loc.x) / (17 * core.text_size)), -floor(double(core.mouse_pos.y - core.top_left_num_loc.y) / (15 * core.text_size))};
    hovered_num.y += core.start_loc;
    if(hovered_num != core.hovered_num) {
        core.hovered_num = hovered_num;
    }

    if(core.scrollbar.clicked_on && core.mouse_pos.y < core.scrollbar.length && core.mouse_pos.y >= 0) {
        if(core.mouse_pos.y == 0) {
            core.scrollbar.position = 0.0;
        } else if(core.mouse_pos.y == core.scrollbar.length) {
            core.scrollbar.position = 1.0;
        } else {
            int y_dif = core.mouse_pos.y - prev_pos.y;

            double scaled_dif = double(y_dif) / (core.scrollbar.length - core.scrollbar.bar.size.y);

            core.scrollbar.position += scaled_dif;
            core.scrollbar.position = std::clamp(core.scrollbar.position, 0.0, 1.0);

            core.scrollbar.set_val(core.start_loc, true);
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    if(core.mouse_button_map.contains(button)) {
        if(action == GLFW_PRESS) {
            core.mouse_button_map[button] = true;
        } else if(action == GLFW_RELEASE) {
            core.mouse_button_map[button] = false;
        }
    }

    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        if(action == GLFW_PRESS) {
            if(core.rows_total > core.rows_shown && core.scrollbar.bar.contains(core.mouse_pos)) {
                core.scrollbar.clicked_on = true;
            }

            if(core.hovered_num.y >= 0 && core.hovered_num.y <= core.bytes.size() / 16 && core.hovered_num.x >= 0 && core.hovered_num.x < 16) {
                if(core.hovered_num.x + core.hovered_num.y * 16 <= core.bytes.size()) {
                    if(core.input_status != -1) {
                        core.byte_rows[core.selected_num.y].load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                    }

                    if(core.selected_num != core.hovered_num) {
                        core.selected_num = core.hovered_num;
                    } else {
                        core.selected_num.y = -1;
                    }

                    core.input_status = -1;
                }
            } else if(core.selected_num.y != -1 && !core.scrollbar.clicked_on) {
                core.byte_rows[core.selected_num.y].load_buffers(core.font, core.text_size, core.selected_num.y * 0x10, core.bytes, core.ten);
                core.selected_num.y = -1;
                core.input_status = -1;
            }
        } else if(action == GLFW_RELEASE) {
            core.scrollbar.clicked_on = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    int new_loc = core.start_loc - (yoffset * 4);

    if(new_loc < 0) {
        core.start_loc = 0;
    } else if(new_loc > core.scrollbar.max) {
        core.start_loc = core.scrollbar.max;
    } else {
        core.start_loc -= (yoffset * 4);
    }

    core.scrollbar.update_pos(core.start_loc, true);

    glm::ivec2 hovered_num = {floor(double(core.mouse_pos.x - core.top_left_num_loc.x) / (17 * core.text_size)), -floor(double(core.mouse_pos.y - core.top_left_num_loc.y) / (15 * core.text_size))};
    hovered_num.y += core.start_loc;
    if(hovered_num != core.hovered_num) {
        core.hovered_num = hovered_num;
    }
}

void Core::game_loop() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat3 view_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2)));
    view_matrix = glm::translate(view_matrix, glm::vec2(-viewport_size / 2));

    update_dimensions();
    
    glm::vec2 translate_pos = glm::vec2(20, screen_size.y - 28 * text_size - 56);

    glm::mat3 transform_matrix = glm::translate(identity_matrix, translate_pos);

    glm::mat3 num_matrix = glm::translate(identity_matrix, glm::vec2(20 + 63 * text_size, screen_size.y - 13 * text_size - 56));


    shaders[0].use();
    textures[0].bind(0);

    num_buffer.bind();
    num_color_buffer.bind(0);
    glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &num_matrix[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, num_buffer.vertices);

    for(int i = start_loc; i < end_loc; i++) {
        Text_row& t = byte_rows[i];

        if(!t.vertex_buf.initialized) {
            t.init_buffers();
            t.load_buffers(font, text_size, i * 0x10, bytes, ten);
        }

        t.vertex_buf.bind();
        t.color_buf.bind(0);

        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, t.vertex_buf.vertices);

        translate_pos += glm::vec2(0, -15 * text_size);

        transform_matrix = glm::translate(identity_matrix, translate_pos);
    }

    shaders[1].use();
    buffers[0].bind();

    if(selected_num.y != -1 && selected_num.y >= start_loc && selected_num.y < end_loc) {
        glm::mat3 select_matrix = glm::translate(identity_matrix, glm::vec2(top_left_num_loc + (glm::ivec2{17, -15} * glm::ivec2(selected_num.x, selected_num.y - start_loc)) * text_size));
        select_matrix = glm::scale(select_matrix, glm::vec2{17, 15} * float(text_size));

        glUniformMatrix3fv(0, 1, false, &select_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
        glUniform4f(2, 0.0f, 1.0f, 0.0f, 0.25f);
        glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
    }
    if(hovered_num != selected_num && hovered_num.y >= start_loc && hovered_num.y < end_loc && hovered_num.x >= 0 && hovered_num.x < 16 && hovered_num.y * 16 + hovered_num.x <= bytes.size()) {
        glm::mat3 hover_matrix = glm::translate(identity_matrix, glm::vec2(top_left_num_loc + (glm::ivec2{17, -15} * glm::ivec2(hovered_num.x, hovered_num.y - start_loc)) * text_size));
        hover_matrix = glm::scale(hover_matrix, glm::vec2{17, 15} * float(text_size));

        glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
        glUniform4f(2, 0.0f, 1.0f, 0.0f, 0.125f);
        glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
    }

    glm::mat3 scrollbar_matrix = glm::scale(identity_matrix, {10, scrollbar.length});

    glUniformMatrix3fv(0, 1, false, &scrollbar_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
    glUniform4f(2, 0.0f, 1.0f, 0.0f, 0.25f);
    glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);

    if(rows_total > rows_shown) {
        scrollbar_matrix = glm::translate(identity_matrix, glm::vec2(scrollbar.bar.position));
        scrollbar_matrix = glm::scale(scrollbar_matrix, glm::vec2(scrollbar.bar.size));
        glUniformMatrix3fv(0, 1, false, &scrollbar_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
        glUniform4f(2, 0.0f, 1.0f, 0.0f, 0.5f);
        glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
    }

    glm::mat3 line_matrix = glm::translate(identity_matrix, {0, screen_size.y - 52});
    line_matrix = glm::scale(line_matrix, {screen_size.x, 2});
    glUniformMatrix3fv(0, 1, false, &line_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
    glUniform4f(2, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);


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
    glfwSetCursorPosCallback(core.window, cursor_pos_callback);
    glfwSetMouseButtonCallback(core.window, mouse_button_callback);
    glfwSetScrollCallback(core.window, scroll_callback);

    int bytes_total = 0x1000;
    core.bytes = std::vector<uint8_t>(bytes_total);
    for(int i = 0; i < bytes_total; i++) {
        core.bytes[i] = rand() % 256;
    }
    
    core.load_assets();
    core.load_byte_rows();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }

    glfwTerminate();
}