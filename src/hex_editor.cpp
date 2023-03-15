#define USE_2D
#include "wrapper.cpp"
#include <fstream>
#include <sstream>
#include <windows.h>
#include <string>

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

std::string open_dialog() {
    char filename[MAX_PATH];

    OPENFILENAMEA ofn;
    ZeroMemory(&filename, MAX_PATH);
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = FindWindowA(NULL, "Avery's Hex Editor");
    ofn.lpstrFilter = "Any file (.*)\0*.*\0Binary files (.bin)\0*.bin\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if(!GetOpenFileNameA(&ofn)) {
        uint32_t error = CommDlgExtendedError();
        if(error != 0) {
            std::cout << "there was an error (GetOpenFileNameA): 0x" << std::hex << error << "\n";
        }
    }

    return std::string(ofn.lpstrFile);
}

std::string save_as_dialog() {
    char filename[MAX_PATH];

    OPENFILENAMEA ofn;
    ZeroMemory(&filename, MAX_PATH);
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = FindWindowA(NULL, "Avery's Hex Editor");
    ofn.lpstrFilter = "Any file (.*)\0*.*\0Binary files (.bin)\0*.bin\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_DONTADDTORECENT;

    if(!GetSaveFileNameA(&ofn)) {
        uint32_t error = CommDlgExtendedError();
        if(error != 0) {
            std::cout << "there was an error (GetOpenFileNameA): 0x" << std::hex << error << "\n";
        }
    }

    return std::string(ofn.lpstrFile);
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

std::vector<uint8_t> get_bytes_from_file(char* path) {
    std::vector<uint8_t> bytes;

    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary);

    if(file.is_open()) {
        file.seekg(0, std::ios::end);
        int size = file.tellg();
        file.seekg(0, std::ios::beg);

        bytes = std::vector<uint8_t>(size);
        file.read((char*)bytes.data(), size);
    } else {
        std::cout << "failed to open file " << path << std::endl;
    }

    file.close();

    return bytes;
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

void insert_char(std::vector<Vertex>& vertices, Font_data& font, int size, Glyph_data& g, glm::ivec2 pos) {
    vertices.push_back({{pos.x, pos.y}, {g.tex_coord, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y + font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});
}

struct Text_row {
    Buffer vertex_buf;

    void init_buffers() {
        vertex_buf.init();
    }

    void clear_buffers() {
        vertex_buf.delete_buffers();
    }

    void load_buffers(bool text_mode, Font_data& font, int size, uint32_t line_num, std::vector<uint8_t>& bytes, uint8_t ten) {
        std::vector<Vertex> vertices;

        int pos = 0;
        for(int i = 7; i >= 0; i--) {
            uint8_t n = (line_num >> (i * 4)) & 0xF;
            Glyph_data& g = font.glyph_map[(n < 0xA) ? 0x30 + n : ten - 10 + n];

            insert_char(vertices, font, size, g, {pos, 0});
            pos += g.stride * size;
        }

        pos += 12 * size;

        int bytes_num = std::min(16u, (unsigned int)bytes.size() - line_num);

        if(text_mode) {
            for(int i = line_num; i < line_num + bytes_num; ++i) {
                Glyph_data g;
                if(font.glyph_map.contains(bytes[i])) g = font.glyph_map[bytes[i]];
                else g = font.glyph_map[0x88];

                if(g.visible) insert_char(vertices, font, size, g, {pos + (5 - g.tex_width / 2) * size, 0});

                pos += 17 * size;
            }
        } else {
            for(int i = line_num; i < line_num + bytes_num; ++i) {
                uint8_t byte = bytes[i];
                uint8_t a = byte >> 4;
                uint8_t b = byte & 0xF;
                Glyph_data& ga = font.glyph_map[(a < 0xA) ? 0x30 + a : ten - 10 + a];
                Glyph_data& gb = font.glyph_map[(b < 0xA) ? 0x30 + b : ten - 10 + b];

                insert_char(vertices, font, size, ga, {pos, 0});
                pos += ga.stride * size;

                insert_char(vertices, font, size, gb, {pos, 0});
                pos += (gb.stride + 5) * size;
            }
        }

        if(bytes_num < 16) {
            Glyph_data& g = font.glyph_map['+'];

            pos += ((font.glyph_map['0'].stride * 2 - 1) - g.tex_width) / 2 * size;

            insert_char(vertices, font, size, g, {pos, 0});
        }

        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);
    }

    void load_buffers(bool text_mode, Font_data& font, int size, uint32_t line_num, std::vector<uint8_t>& bytes, uint8_t ten, int missing_index, int show) {
        std::vector<Vertex> vertices;
        std::vector<glm::vec4> colors;

        int pos = 0;
        for(int i = 7; i >= 0; i--) {
            uint8_t n = (line_num >> (i * 4)) & 0xF;
            Glyph_data& g = font.glyph_map[(n < 0xA) ? 0x30 + n : ten - 10 + n];

            insert_char(vertices, font, size, g, {pos, 0});
            pos += g.stride * size;
        }

        pos += 12 * size;

        int bytes_num = std::min(16u, (unsigned int)bytes.size() - line_num);

        int i;

        if(text_mode) {
            for(i = line_num; i < line_num + bytes_num; ++i) {
                uint8_t byte = bytes[i];
                if(i - line_num == missing_index) {
                    if(show == 1) {
                        uint8_t a = bytes[i] >> 4;
                        Glyph_data& ga = font.glyph_map[(a < 0xA) ? 0x30 + a : ten - 10 + a];

                        insert_char(vertices, font, size, ga, {pos, 0});
                    }
                    pos += (font.glyph_map['0'].stride * 2 + 5) * size;
                } else {
                    Glyph_data g;
                    if(font.glyph_map.contains(bytes[i])) g = font.glyph_map[bytes[i]];
                    else g = font.glyph_map[0x88];

                    if(g.visible) insert_char(vertices, font, size, g, {pos + (5 - g.tex_width / 2) * size, 0});

                    pos += 17 * size;
                }
            }
        } else {
            for(i = line_num; i < line_num + bytes_num; ++i) {
                uint8_t byte = bytes[i];
                if(i - line_num == missing_index) {
                    if(show == 1) {
                        uint8_t a = bytes[i] >> 4;
                        Glyph_data& ga = font.glyph_map[(a < 0xA) ? 0x30 + a : ten - 10 + a];

                        insert_char(vertices, font, size, ga, {pos, 0});
                    }
                    pos += (font.glyph_map['0'].stride * 2 + 5) * size;
                } else {
                    uint8_t byte = bytes[i];

                    uint8_t a = byte >> 4;
                    uint8_t b = byte & 0xF;
                    Glyph_data& ga = font.glyph_map[(a < 0xA) ? 0x30 + a : ten - 10 + a];
                    Glyph_data& gb = font.glyph_map[(b < 0xA) ? 0x30 + b : ten - 10 + b];

                    insert_char(vertices, font, size, ga, {pos, 0});
                    pos += ga.stride * size;

                    insert_char(vertices, font, size, gb, {pos, 0});
                    pos += (gb.stride + 5) * size;
                }
            }
        }

        if(bytes_num < 16 && missing_index != i) {
            Glyph_data& g = font.glyph_map['+'];

            pos += ((font.glyph_map['0'].stride * 2 - 1) - g.tex_width) / 2 * size;

            insert_char(vertices, font, size, g, {pos, 0});
        }

        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);
    }
};

struct Text {
    Buffer vertex_buf;
    glm::ivec2 size;

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

struct Button {
    AABB box;
    Text text;
};

struct Scrollbar {
    int length;
    AABB bar;
    double position = 0.0;
    bool clicked_on = false;

    Scrollbar(double pos) {
        position = pos;
    }

    void update_pos(double new_pos, bool inverse) {
        if(inverse) {
            position = 1.0 - double(new_pos);
            position = std::clamp(position, 0.0, 1.0);
            bar.position.y = length - (1.0 - position) * (length - bar.size.y) - bar.size.y;
        } else {
            position = double(new_pos);
            position = std::clamp(position, 0.0, 1.0);
            bar.position.y = length - position * (length - bar.size.y) - bar.size.y;
        }
    }
};

struct Tab {
    std::string path;
    Text text;
    std::vector<uint8_t> bytes;
    bool edited = false;

    int pos = 0;
    glm::ivec2 selected = {0, -1};
};

enum screens {EDITOR, TABS, SETTINGS, DELETE_TAB};

struct Core {
    std::map<GLuint, bool> mouse_button_map = {
        {GLFW_MOUSE_BUTTON_LEFT, false}
    };

    std::map<GLuint, bool> key_map = {
        {GLFW_KEY_LEFT_CONTROL, false}  
    };

    std::vector<Tab> tabs;
    int active_tab = 0;

    uint8_t ten = 0x80;

    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};
    glm::ivec2 cursor_pos = {0, 0};

    std::vector<Shader> shaders;
    std::vector<Texture> textures;
    std::vector<Buffer> buffers;
    std::vector<Storage_buffer> storage_buffers;
    std::vector<Text> texts;

    Font_data font;
    int text_size = 2;
    
    Scrollbar scrollbar = Scrollbar(1.0);

    std::map<uint32_t, Text_row> byte_rows;
    int start_loc = 0;
    int end_loc = 0;
    int rows_shown = 0;
    int rows_total = 0;

    glm::ivec2 hovered_num = {0, 0};
    glm::ivec2 selected_num = {0, -1};
    int input_status = 2;

    glm::vec4 color = {0.0f, 1.0f, 0.0f, 1.0f};

    std::vector<Button> buttons;
    std::vector<int> visible_buttons = {0, 1, 2, 3, 4, 5, 6};
    std::vector<int> active_buttons = {1, 2, 3, 4, 5, 6};
    /*
    0 -> editor
    1 -> tabs
    2 -> new
    3 -> open
    4 -> save
    5 -> save as
    6 -> settings
    */

    bool big_endian = false;
    bool text_mode = false;

    screens active_screen = EDITOR;
    int tab_delete = 0;

    bool edit_color = false;
    std::string color_string = "";

    void load_font() {
        std::ifstream file;
        file.open("res\\text_data.bin", std::ios::in | std::ios::binary);

        if(file.is_open()) {
            file.read((char*)&font.line_height, 1);
            file.read((char*)&font.line_spacing, 1);

            uint16_t space_glyphs;
            file.read((char*)&space_glyphs, 2);

            for(int i = 0; i < space_glyphs; ++i) {
                uint8_t id;
                Glyph_data data;

                file.read((char*)&id, 1);
                file.read((char*)&data.stride, 1);

                data.visible = false;

                font.glyph_map.insert({id, data});
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

                font.glyph_map.insert({id, data});
            }

            file.close();
        } else {
            std::cout << "error" << std::endl;
        }
    }

    void load_assets() {
        shaders = std::vector<Shader>(3);
        textures = std::vector<Texture>(2);
        buffers = std::vector<Buffer>(7);
        texts = std::vector<Text>(3);
        storage_buffers = std::vector<Storage_buffer>(2);
        
        shaders[0].compile(get_text_from_file("res\\shaders\\text_color.vs").data(), get_text_from_file("res\\shaders\\text_color.fs").data());
        shaders[1].compile(get_text_from_file("res\\shaders\\select.vs").data(), get_text_from_file("res\\shaders\\select.fs").data());
        shaders[2].compile(get_text_from_file("res\\shaders\\text.vs").data(), get_text_from_file("res\\shaders\\text.fs").data());

        textures[0].load("res\\text.png");
        textures[1].load("res\\cursor.png");
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

        std::vector<Vertex> vertex_vec1;
        vertex_vec1.push_back({{0, 0}, {0, 0}});
        vertex_vec1.push_back({{1, 0}, {16, 0}});
        vertex_vec1.push_back({{0, 1}, {0, 16}});
        vertex_vec1.push_back({{0, 1}, {0, 16}});
        vertex_vec1.push_back({{1, 0}, {16, 0}});
        vertex_vec1.push_back({{1, 1}, {16, 16}});

        buffers[6].init();
        buffers[6].set_data(vertex_vec1.data(), vertex_vec1.size(), sizeof(Vertex));
        buffers[6].set_attrib(0, 2, sizeof(float) * 4, 0);
        buffers[6].set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        buffers[2].init();
        storage_buffers[1].init();

        scrollbar.bar.size.x = 10;
        scrollbar.bar.position.x = 0;
    }

    void load_info_buffer(glm::ivec2 selected) {
        int index = selected.y * 16 + selected.x;

        std::vector<Vertex> vertex_vec;
        std::vector<glm::vec4> color_vec;

        glm::ivec2 pos = {0, 0};



        std::string str = "binary: ";
        for(int i = 7; i >= 0; i--) {
            if((tabs[active_tab].bytes[index] >> i) & 1) str += '1';
            else str += '0';
        }

        for(char c : str) {
            Glyph_data& g = font.glyph_map[c];

            if(g.visible) {
                insert_char(vertex_vec, font, text_size, g, pos);
                color_vec.push_back(color);
            }
            pos.x += g.stride * text_size;
        }
        
        pos.x = 0;
        pos.y -= (font.line_spacing + 5) * text_size;



        str = "uint8: " + std::to_string(tabs[active_tab].bytes[index]);

        for(char c : str) {
            Glyph_data& g = font.glyph_map[c];

            if(g.visible) {
                insert_char(vertex_vec, font, text_size, g, pos);
                color_vec.push_back(color);
            }
            pos.x += g.stride * text_size;
        }

        pos.x = 0;
        pos.y -= (font.line_spacing + 5) * text_size;



        str = "int8: " + std::to_string(int8_t(tabs[active_tab].bytes[index]));

        for(char c : str) {
            Glyph_data& g = font.glyph_map[c];

            if(g.visible) {
                insert_char(vertex_vec, font, text_size, g, pos);
                color_vec.push_back(color);
            }
            pos.x += g.stride * text_size;
        }

        pos.x = 0;
        pos.y -= (font.line_spacing + 5) * text_size;

        if(!big_endian) {
            uint16_t num16 = 0;
            num16 |= tabs[active_tab].bytes[index];
            num16 |= tabs[active_tab].bytes[index + 1] << 8;

            uint32_t num32 = uint32_t(num16);
            for(int i = 2; i < 4; ++i) {
                num32 |= uint32_t(tabs[active_tab].bytes[index + i]) << (8 * i);
            }

            uint64_t num64 = uint64_t(num32);
            for(int i = 4; i < 8; ++i) {
                num64 |= uint64_t(tabs[active_tab].bytes[index + i]) << (8 * i);
            }



            str = "uint16: ";
            if(index + 1 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(num16);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "int16: ";
            if(index + 1 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(*(int16_t*)&num16);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "uint32: ";
            if(index + 3 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(num32);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "int32: ";
            if(index + 3 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(*(int32_t*)&num32);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;
            


            str = "uint64: ";
            if(index + 7 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(num64);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "int64: ";
            if(index + 7 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(*(int64_t*)&num64);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * 2 * text_size;


            
            str = "float32: ";
            if(index + 3 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                float f = *(float*)&num32;
                std::stringstream sstream;
                sstream << f;
                str += sstream.str();
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;


            str = "float64: ";
            if(index + 7 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                double d = *(double*)&num64;
                std::stringstream sstream;
                sstream << d;
                str += sstream.str();
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * 2 * text_size;



            str = "char: ";
            if(font.glyph_map.contains(tabs[active_tab].bytes[index])) {
                if(font.glyph_map[tabs[active_tab].bytes[index]].visible) str += tabs[active_tab].bytes[index];
                else str += "[invisible]";
            } else {
                str += "[?]";
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "color: ";
            if(index + 2 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";

                for(char c : str) {
                    Glyph_data& g = font.glyph_map[c];

                    if(g.visible) {
                        insert_char(vertex_vec, font, text_size, g, pos);
                        color_vec.push_back(color);
                    }
                    pos.x += g.stride * text_size;
                }
            } else {
                str += 0x87;

                for(char c : str) {
                    Glyph_data& g = font.glyph_map[c];

                    if(g.visible) {
                        insert_char(vertex_vec, font, text_size, g, pos);
                        color_vec.push_back(color);
                    }
                    pos.x += g.stride * text_size;
                }

                color_vec[color_vec.size() - 1] = {float(tabs[active_tab].bytes[index]) / 255, float(tabs[active_tab].bytes[index + 1]) / 255, float(tabs[active_tab].bytes[index + 2]) / 255, 1.0f};
            }
        } else {
            uint16_t num16 = 0;
            num16 |= tabs[active_tab].bytes[index] << 8;
            num16 |= tabs[active_tab].bytes[index + 1];

            uint32_t num32 = uint32_t(num16) << 16;
            for(int i = 2; i < 4; ++i) {
                num32 |= uint32_t(tabs[active_tab].bytes[index + i]) << (8 * (3 - i));
            }

            uint64_t num64 = uint64_t(num32) << 32;
            for(int i = 4; i < 8; ++i) {
                num64 |= uint64_t(tabs[active_tab].bytes[index + i]) << (8 * (7 - i));
            }

            str = "uint16: ";
            if(index + 1 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(num16);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "int16: ";
            if(index + 1 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(*(int16_t*)&num16);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "uint32: ";
            if(index + 3 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(num32);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "int32: ";
            if(index + 3 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(*(int32_t*)&num32);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;
            


            str = "uint64: ";
            if(index + 7 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(num64);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "int64: ";
            if(index + 7 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                str += std::to_string(*(int64_t*)&num64);
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * 2 * text_size;


            
            str = "float32: ";
            if(index + 3 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                float f = *(float*)&num32;
                std::stringstream sstream;
                sstream << f;
                str += sstream.str();
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "float64: ";
            if(index + 7 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";
            } else {
                double d = *(double*)&num64;
                std::stringstream sstream;
                sstream << d;
                str += sstream.str();
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * 2 * text_size;



            str = "char: ";
            if(font.glyph_map.contains(tabs[active_tab].bytes[index])) {
                if(font.glyph_map[tabs[active_tab].bytes[index]].visible) str += tabs[active_tab].bytes[index];
                else str += "[invisible]";
            } else {
                str += "[?]";
            }

            for(char c : str) {
                Glyph_data& g = font.glyph_map[c];

                if(g.visible) {
                    insert_char(vertex_vec, font, text_size, g, pos);
                    color_vec.push_back(color);
                }
                pos.x += g.stride * text_size;
            }

            pos.x = 0;
            pos.y -= (font.line_spacing + 5) * text_size;



            str = "color: ";
            if(index + 2 >= tabs[active_tab].bytes.size()) {
                str += "[EOF]";

                for(char c : str) {
                    Glyph_data& g = font.glyph_map[c];

                    if(g.visible) {
                        insert_char(vertex_vec, font, text_size, g, pos);
                        color_vec.push_back(color);
                    }
                    pos.x += g.stride * text_size;
                }
            } else {
                str += 0x87;

                for(char c : str) {
                    Glyph_data& g = font.glyph_map[c];

                    if(g.visible) {
                        insert_char(vertex_vec, font, text_size, g, pos);
                        color_vec.push_back(color);
                    }
                    pos.x += g.stride * text_size;
                }

                color_vec[color_vec.size() - 1] = {float(tabs[active_tab].bytes[index]) / 255, float(tabs[active_tab].bytes[index + 1]) / 255, float(tabs[active_tab].bytes[index + 2]) / 255, 1.0f};
            }
        }

        buffers[2].set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex));
        buffers[2].set_attrib(0, 2, sizeof(float) * 4, 0);
        buffers[2].set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        storage_buffers[1].set_data(color_vec.data(), color_vec.size() * sizeof(glm::vec4));
    }

    void reset_total_rows() {
        if(active_screen == TABS) {
            rows_total = tabs.size();
            rows_shown = (screen_size.y - 30 * text_size) / (15 * text_size) + 1;
        } else {
            rows_total = tabs[active_tab].bytes.size() / 16 + 1;
            rows_shown = (screen_size.y - 45 * text_size) / (15 * text_size) + 1;
        }

        int bar_size = std::max(double(rows_shown) / rows_total * scrollbar.length, 20.0);
        scrollbar.update_pos(double(start_loc) / std::max(rows_total - rows_shown, 1), true);
        
        scrollbar.bar = {{10, bar_size}, {0, scrollbar.length - (1.0 - scrollbar.position) * (scrollbar.length - bar_size) - bar_size}};
    }
    
    void resize_screen() {
        scrollbar.length = screen_size.y - 15 * text_size;
        reset_total_rows();

        for(int i = 0; i < 7; ++i) {
            buttons[i].box.position.y = screen_size.y - buttons[i].box.size.y;
        }

        for(int i = 7; i < 10; ++i) {
            buttons[i].box.position.y = screen_size.y - 45 * text_size;
        }

        buttons[10].box.position.y = screen_size.y - 30 * text_size;
        buttons[11].box.position.y = screen_size.y - 45 * text_size;
        buttons[12].box.position.y = screen_size.y - 60 * text_size;
    }

    void load_byte_rows() {
        std::vector<Vertex> vertex_vec;
        std::vector<glm::vec4> color_vec;
        
        int pos = 0;
        for(int n = 0; n < 0x10; n++) {
            Glyph_data& g = font.glyph_map[(n < 0xA) ? 0x30 + n : ten - 10 + n];

            insert_char(vertex_vec, font, text_size, g, {pos, 0});

            color_vec.push_back(color);

            pos += 17 * text_size;
        }

        buffers[1].init();
        storage_buffers[0].init();

        buffers[1].set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex));
        buffers[1].set_attrib(0, 2, sizeof(float) * 4, 0);
        buffers[1].set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        storage_buffers[0].set_data(color_vec.data(), color_vec.size() * sizeof(glm::vec4));

        Text t;
        t.init_buffers();
        t.load_buffers(font, "Save file before closing?", text_size);
        buffers[3] = std::move(t.vertex_buf);

        buffers[4].init();
        std::vector<Vertex> v;
        insert_char(v, font, text_size, font.glyph_map['x'], {0, 0});
        buffers[4].set_data(v.data(), 6, sizeof(Vertex));
        buffers[4].set_attrib(0, 2, sizeof(float) * 4, 0);
        buffers[4].set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        buffers[5].init();
        v.clear();
        insert_char(v, font, text_size, font.glyph_map['?'], {0, 0});
        buffers[5].set_data(v.data(), 6, sizeof(Vertex));
        buffers[5].set_attrib(0, 2, sizeof(float) * 4, 0);
        buffers[5].set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        texts[0].init_buffers();
        texts[0].load_buffers(font, "Byte Mode:", text_size);

        texts[1].init_buffers();
        texts[1].load_buffers(font, "Color:", text_size);

        texts[2].init_buffers();
        texts[2].load_buffers(font, "Endianness:", text_size);

        buttons = std::vector<Button>(13);

        for(Button& b : buttons) {
            b.text.init_buffers();
        }

        buttons[0].text.load_buffers(font, "Editor", text_size);
        buttons[1].text.load_buffers(font, "Tabs", text_size);
        buttons[2].text.load_buffers(font, "New", text_size);
        buttons[3].text.load_buffers(font, "Open", text_size);
        buttons[4].text.load_buffers(font, "Save", text_size);
        buttons[5].text.load_buffers(font, "Save As", text_size);
        buttons[6].text.load_buffers(font, "Settings", text_size);

        buttons[7].text.load_buffers(font, "Save", text_size);
        buttons[8].text.load_buffers(font, "Don't Save", text_size);
        buttons[9].text.load_buffers(font, "Cancel", text_size);
        buttons[10].text.load_buffers(font, "Hex", text_size);
        buttons[11].text.load_buffers(font, "00\u0085\u008500", text_size);
        buttons[12].text.load_buffers(font, "Little", text_size);

        glm::ivec2 position = {0, 0};

        for(int i = 0; i < 7; ++i) {
            Button& b = buttons[i];
            b.box = {b.text.size + glm::ivec2{10, 6} * text_size, position};
            position.x += b.box.size.x;
        }

        position = {0, 0};

        for(int i = 7; i < 10; ++i) {
            Button& b = buttons[i];
            b.box = {b.text.size + glm::ivec2{10, 6} * text_size, position};
            position.x += b.box.size.x;
        }
    
        buttons[10].box = {{glm::ivec2{23 + 10, 9 + 6} * text_size}, {texts[0].size.x + 10 * text_size, 0}};
        buttons[11].box = {{buttons[11].text.size + glm::ivec2{10, 6} * text_size}, {texts[1].size.x + 10 * text_size, 0}};
        buttons[12].box = {{buttons[12].text.size + glm::ivec2{10, 6} * text_size}, {texts[2].size.x + 10 * text_size, 0}};
        
        tabs.push_back(Tab());
        tabs[0].text.init_buffers();
        tabs[0].text.load_buffers(font, "New file", text_size);

        resize_screen();
    }

    void change_active_screen(screens new_screen) {
        if(active_screen == EDITOR && input_status != -1) {
            input_status = -1;
            byte_rows[selected_num.y].load_buffers(text_mode, font, text_size, selected_num.y << 4, tabs[active_tab].bytes, ten);
        } else if(active_screen == SETTINGS && edit_color) {
            edit_color = false;

            int add_zeroes = 6 - color_string.size();
            for(int i = 0; i < add_zeroes; ++i) {
                color_string += '0';
            }

            buttons[11].text.load_buffers(font, color_string, text_size);
            if(color_string.size() >= 6) {
                int col = std::stoi(color_string, 0, 16);
                color = {float((col >> 16) & 0xFF) / 255, float((col >> 8) & 0xFF) / 255, float(col & 0xFF) / 255, 1.0f};
                if(selected_num.y * 16 + selected_num.x < tabs[active_tab].bytes.size()) load_info_buffer(selected_num);
                edit_color = false;
            }
        }

        switch(new_screen) {
            case EDITOR: {
                active_screen = EDITOR;

                visible_buttons = {0, 1, 2, 3, 4, 5, 6};
                active_buttons = {1, 2, 3, 4, 5, 6};

                start_loc = tabs[active_tab].pos;
                selected_num = tabs[active_tab].selected;

                reset_total_rows();
                break;
            }
            case TABS: {
                active_screen = TABS;

                visible_buttons = {0, 1, 2, 3, 4, 5, 6};
                active_buttons = {0, 2, 3, 4, 5, 6};

                tabs[active_tab].pos = start_loc;
                tabs[active_tab].selected = selected_num;
                selected_num.y = active_tab;
                start_loc = 0;

                reset_total_rows();
                break;
            }
            case SETTINGS: {
                active_screen = SETTINGS;

                visible_buttons = {0, 1, 2, 3, 4, 5, 6, 10, 11, 12};
                active_buttons = {0, 1, 2, 3, 4, 5, 10, 11, 12};
                tabs[active_tab].pos = start_loc;
                tabs[active_tab].selected = selected_num;
                
                break;
            }
            case DELETE_TAB: {
                active_screen = DELETE_TAB;

                visible_buttons = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
                active_buttons = {7, 8, 9};

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

    core.resize_screen();

    core.game_loop();
}

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    if(core.edit_color) {
        if(('0' <= codepoint && '9' >= codepoint)) {
            core.color_string += codepoint;

            core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
            if(core.color_string.size() >= 6) {
                int color = to_hex_number(core.color_string);
                core.color = {float((color >> 16) & 0xFF) / 255, float((color >> 8) & 0xFF) / 255, float(color & 0xFF) / 255, 1.0f};
                if(core.selected_num.y * 16 + core.selected_num.x < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                core.edit_color = false;
            }
        } else if ('A' <= codepoint && 'F' >= codepoint) {
            core.color_string += (codepoint - 'A') + 0x80;

            core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
            if(core.color_string.size() >= 6) {
                int color = to_hex_number(core.color_string);
                core.color = {float((color >> 16) & 0xFF) / 255, float((color >> 8) & 0xFF) / 255, float(color & 0xFF) / 255, 1.0f};
                if(core.selected_num.y * 16 + core.selected_num.x < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                core.edit_color = false;
            }
        } else if('a' <= codepoint && 'f' >= codepoint) {
            core.color_string += (codepoint - 'a') + 0x80;

            core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
            if(core.color_string.size() >= 6) {
                int color = to_hex_number(core.color_string);
                core.color = {float((color >> 16) & 0xFF) / 255, float((color >> 8) & 0xFF) / 255, float(color & 0xFF) / 255, 1.0f};
                if(core.selected_num.y * 16 + core.selected_num.x < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                core.edit_color = false;
            }
        }
    } else if(core.selected_num.y != -1) {
        if(core.text_mode && (codepoint & 0xFF) == codepoint) {
            if(core.selected_num.y != -1) {
                if(core.input_status == -1) {
                    core.tabs[core.active_tab].bytes.insert(core.tabs[core.active_tab].bytes.begin() + (core.selected_num.y * 16 + core.selected_num.x), (uint8_t)codepoint);
                    core.tabs[core.active_tab].edited = true;
                    core.reset_total_rows();

                    std::vector<uint32_t> delete_keys;
                    for(auto p = core.byte_rows.begin(); p != core.byte_rows.end(); ++p) {
                        int i = p->first;
                        Text_row& t = p->second;
                        if(i >= core.selected_num.y) {
                            if(i >= core.end_loc) {
                                delete_keys.push_back(i);
                            } else {
                                t.load_buffers(core.text_mode, core.font, core.text_size, i * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                            }
                        }
                    }

                    for(uint32_t key : delete_keys) {
                        core.byte_rows.erase(key);
                    }
                }

                ++core.selected_num.x;
                if(core.selected_num.x > 15) {
                    core.selected_num.x = 0;
                    ++core.selected_num.y;
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }
            }
        } else {
            if(codepoint >= '0' && codepoint <= '9') {
                Text_row& row = core.byte_rows[core.selected_num.y];
                if(core.selected_num.y * 16 + core.selected_num.x == core.tabs[core.active_tab].bytes.size()) {
                    core.tabs[core.active_tab].bytes.push_back(0x0);
                    core.tabs[core.active_tab].edited = true;
                    
                    core.reset_total_rows();

                    if(core.tabs[core.active_tab].bytes.size() % 16 == 0) {
                        core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                    }
                }
                uint8_t& byte = core.tabs[core.active_tab].bytes[core.selected_num.y * 16 + core.selected_num.x];
                if(core.input_status == 0 || core.input_status == -1) {
                    byte = (codepoint - '0') << 4;
                    core.tabs[core.active_tab].edited = true;

                    row.load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten, core.selected_num.x, 1);
                    core.input_status = 1;
                } else if(core.input_status == 1) {
                    byte |= codepoint - '0';
                    core.tabs[core.active_tab].edited = true;

                    row.load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;
                    

                    ++core.selected_num.x;
                    if(core.selected_num.x > 15) {
                        core.selected_num.x = 0;
                        ++core.selected_num.y;
                    }
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }

                core.load_info_buffer(core.selected_num);
            } else if(codepoint >= 'a' && codepoint <= 'f') {
                Text_row& row = core.byte_rows[core.selected_num.y];
                if(core.selected_num.y * 16 + core.selected_num.x == core.tabs[core.active_tab].bytes.size()) {
                    core.tabs[core.active_tab].bytes.push_back(0x0);
                    core.tabs[core.active_tab].edited = true;

                    core.reset_total_rows();

                    if(core.tabs[core.active_tab].bytes.size() % 16 == 0) {
                        core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                    }
                }
                uint8_t& byte = core.tabs[core.active_tab].bytes[core.selected_num.y * 16 + core.selected_num.x];
                if(core.input_status == 0 || core.input_status == -1) {
                    byte = (codepoint - 'a' + 0xA) << 4;
                    core.tabs[core.active_tab].edited = true;

                    row.load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten, core.selected_num.x, 1);
                    core.input_status = 1;
                } else if(core.input_status == 1) {
                    byte |= codepoint - 'a' + 0xA;
                    core.tabs[core.active_tab].edited = true;

                    row.load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;
                    
                    ++core.selected_num.x;
                    if(core.selected_num.x > 15) {
                        core.selected_num.x = 0;
                        ++core.selected_num.y;
                    }
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }

                core.load_info_buffer(core.selected_num);
            } else if(codepoint >= 'A' && codepoint <= 'F') {
                Text_row& row = core.byte_rows[core.selected_num.y];
                if(core.selected_num.y * 16 + core.selected_num.x == core.tabs[core.active_tab].bytes.size()) {
                    core.tabs[core.active_tab].bytes.push_back(0x0);
                    core.tabs[core.active_tab].edited = true;

                    core.reset_total_rows();

                    if(core.tabs[core.active_tab].bytes.size() % 16 == 0) {
                        core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                    }
                }
                uint8_t& byte = core.tabs[core.active_tab].bytes[core.selected_num.y * 16 + core.selected_num.x];
                if(core.input_status == 0 || core.input_status == -1) {
                    byte = (codepoint - 'A' + 0xA) << 4;
                    core.tabs[core.active_tab].edited = true;

                    row.load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten, core.selected_num.x, 1);
                    core.input_status = 1;

                } else if(core.input_status == 1) {
                    byte |= codepoint - 'A' + 0xA;
                    core.tabs[core.active_tab].edited = true;

                    row.load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;

                    ++core.selected_num.x;
                    if(core.selected_num.x > 15) {
                        core.selected_num.x = 0;
                        ++core.selected_num.y;
                    }
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }

                core.load_info_buffer(core.selected_num);
            }
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    if(action == GLFW_PRESS) {
        if(core.key_map.contains(key)) {
            core.key_map[key] = true;
        }
    } else if(action == GLFW_RELEASE) {
        if(core.key_map.contains(key)) {
            core.key_map[key] = false;
        }
    }

    if(key == GLFW_KEY_BACKSPACE) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.edit_color) {
                if(core.color_string.size() > 0) {
                    core.color_string.pop_back();
                    core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
                }
            }

            int loc = core.selected_num.y * 16 + core.selected_num.x;
            if(loc == core.tabs[core.active_tab].bytes.size()) {
                if(loc != 0) {
                    core.tabs[core.active_tab].bytes.pop_back();
                    core.tabs[core.active_tab].edited = true;

                    core.reset_total_rows();
                    
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

                    core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);

                    if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                        core.start_loc = core.selected_num.y;
                        core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                    } else if(core.end_loc <= core.selected_num.y) {
                        core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                        core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                    }
                }
            } else if(core.selected_num.y != -1) {
                Text_row& row = core.byte_rows[core.selected_num.y];
                if(core.key_map[GLFW_KEY_LEFT_CONTROL]) {
                    if(loc != 0) {
                        if(core.selected_num.x == 0) {
                            if(core.selected_num.y != 0) {
                                core.selected_num.x = 15;
                                --core.selected_num.y;
                            }
                        } else {
                            --core.selected_num.x;
                        }

                        
                        core.tabs[core.active_tab].bytes.erase(core.tabs[core.active_tab].bytes.begin() + (loc - 1));
                        core.tabs[core.active_tab].edited = true;

                        core.reset_total_rows();

                        std::vector<uint32_t> delete_keys;
                        for(auto p = core.byte_rows.begin(); p != core.byte_rows.end(); ++p) {
                            int i = p->first;
                            Text_row& t = p->second;
                            if(i >= core.selected_num.y) {
                                if(i >= core.end_loc) {
                                    delete_keys.push_back(i);
                                } else {
                                    t.load_buffers(core.text_mode, core.font, core.text_size, i * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                                }
                            }
                        }
                        for(uint32_t key : delete_keys) {
                            core.byte_rows.erase(key);
                        }

                        core.input_status = -1;
                    }
                } else if(core.input_status == 1 || core.input_status == -1) {
                    core.tabs[core.active_tab].bytes[loc] = 0;
                    core.tabs[core.active_tab].edited = true;

                    row.load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten, core.selected_num.x, 0);
                    core.input_status = 0;
                } else if(core.input_status == 0) {
                    core.tabs[core.active_tab].bytes.erase(core.tabs[core.active_tab].bytes.begin() + loc);
                    core.tabs[core.active_tab].edited = true;

                    core.reset_total_rows();
                    
                    std::vector<uint32_t> delete_keys;
                    for(auto p = core.byte_rows.begin(); p != core.byte_rows.end(); ++p) {
                        int i = p->first;
                        Text_row& t = p->second;
                        if(i >= core.selected_num.y) {
                            if(i >= core.end_loc) {
                                delete_keys.push_back(i);
                            } else {
                                t.load_buffers(core.text_mode, core.font, core.text_size, i * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                            }
                        }
                    }
                    for(uint32_t key : delete_keys) {
                        core.byte_rows.erase(key);
                    }

                    if(core.selected_num.x == 0) {
                        if(core.selected_num.y != 0) {
                            core.selected_num.x = 15;
                            --core.selected_num.y;
                        }
                    } else {
                        --core.selected_num.x;
                    }

                    core.input_status = -1;
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }

                core.load_info_buffer(core.selected_num);
            }
        }
    } else if(key == GLFW_KEY_ENTER) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status == -1) {
                    core.tabs[core.active_tab].bytes.insert(core.tabs[core.active_tab].bytes.begin() + (core.selected_num.y * 16 + core.selected_num.x), 0x0);
                    core.tabs[core.active_tab].edited = true;

                    core.reset_total_rows();

                    std::vector<uint32_t> delete_keys;
                    for(auto p = core.byte_rows.begin(); p != core.byte_rows.end(); ++p) {
                        int i = p->first;
                        Text_row& t = p->second;
                        if(i >= core.selected_num.y) {
                            if(i >= core.end_loc) {
                                delete_keys.push_back(i);
                            } else {
                                t.load_buffers(core.text_mode, core.font, core.text_size, i * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                            }
                        }
                    }

                    for(uint32_t key : delete_keys) {
                        core.byte_rows.erase(key);
                    }
                } else {
                    core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;
                }
                ++core.selected_num.x;
                if(core.selected_num.x > 15) {
                    core.selected_num.x = 0;
                    ++core.selected_num.y;
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }
            }
        }
    } else if(key == GLFW_KEY_ESCAPE) {
        if(action == GLFW_PRESS) {
            if(core.active_screen == EDITOR) {
                if(core.selected_num.y != -1) {
                    if(core.input_status != -1) {
                        core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                        core.input_status = -1;
                    }
                    core.selected_num.y = -1;
                }
            } else {
                core.change_active_screen(EDITOR);
            }
        }
    } else if(key == GLFW_KEY_UP) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;
                }

                glm::ivec2 new_select = {core.selected_num.x, std::max(core.selected_num.y - 1, 0)};

                if(new_select != core.selected_num) {
                    core.selected_num = new_select;
                    core.load_info_buffer(core.selected_num);
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }

                core.load_info_buffer(core.selected_num);
            }
        }
    } else if(key == GLFW_KEY_DOWN) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;
                }

                glm::ivec2 new_select = {core.selected_num.x, std::min(core.selected_num.y + 1, core.rows_total - 1)};
                if(new_select.y * 16 + new_select.x > core.tabs[core.active_tab].bytes.size()) {
                    new_select.x = core.tabs[core.active_tab].bytes.size() % 16;
                }

                if(new_select != core.selected_num) {
                    core.selected_num = new_select;
                    if(core.selected_num.y * 16 + core.selected_num.x != core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                }

                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }

                core.load_info_buffer(core.selected_num);
            }
        }
    } else if(key == GLFW_KEY_RIGHT) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;
                }

                glm::ivec2 new_select = core.selected_num;
                if(new_select.y != core.rows_total - 1 || new_select.x + 1 <= core.tabs[core.active_tab].bytes.size() % 16) {
                    ++new_select.x;
                }
                if(new_select.x > 15) {
                    new_select.x = 0;
                    ++new_select.y;
                }

                if(new_select != core.selected_num) {
                    core.selected_num = new_select;
                    if(core.selected_num.y * 16 + core.selected_num.x != core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                }
                
                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }

                core.load_info_buffer(core.selected_num);
            }
        }
    } else if(key == GLFW_KEY_LEFT) {
        if(action == GLFW_PRESS || action == GLFW_REPEAT) {
            if(core.selected_num.y != -1) {
                if(core.input_status != -1) {
                    core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                    core.input_status = -1;
                }

                glm::ivec2 new_select = core.selected_num - glm::ivec2{1, 0};
                if(new_select.x < 0) {
                    if(new_select.y == 0) {
                        new_select.x = 0;
                    } else {
                        new_select.x = 15;
                        --new_select.y;
                    }
                }

                if(new_select != core.selected_num) {
                    core.selected_num = new_select;
                    core.load_info_buffer(core.selected_num);
                }
                
                if(core.start_loc > core.selected_num.y && core.selected_num.y != -1) { // flag
                    core.start_loc = core.selected_num.y;
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                } else if(core.end_loc <= core.selected_num.y) {
                    core.start_loc = std::max(0, core.selected_num.y - core.rows_shown + 1); // flag
                    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);
                }
            }
        }
    }
    
    
    
    /* else if(key == GLFW_KEY_ENTER) {
        if(core.selected_num.y != -1) {
            Text_row& row = core.byte_rows[core.selected_num.y];
            row.load_buffers(core.text_mode, core.font, core.color, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes);
            core.input_status = 2;
            core.selected_num.y = -1;
        }
    }*/
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    glm::ivec2 prev_pos = core.cursor_pos;
    core.cursor_pos = {xpos, core.screen_size.y - ceil(ypos) - 1};

    if(core.active_screen == EDITOR) {
        core.hovered_num = {floor(double(core.cursor_pos.x - (20 + 57 * core.text_size)) / (17 * core.text_size)), -floor(double(core.cursor_pos.y - (core.screen_size.y - 45 * core.text_size)) / (15 * core.text_size))};
    } else if(core.active_screen == TABS) {
        core.hovered_num = {floor(double(core.cursor_pos.x - (20 + 57 * core.text_size)) / (17 * core.text_size)), -floor(double(core.cursor_pos.y - (core.screen_size.y - 30 * core.text_size)) / (15 * core.text_size))};
    }
    core.hovered_num.y += core.start_loc;

    if(core.scrollbar.clicked_on && core.cursor_pos.y < core.scrollbar.length && core.cursor_pos.y >= 0) {
        if(core.cursor_pos.y >= 0 && prev_pos.y < 0) {
            core.scrollbar.update_pos(1.0, true);
        } else if(core.cursor_pos.y < core.scrollbar.length && prev_pos.y >= core.scrollbar.length) {
            core.scrollbar.update_pos(0.0, true);
        } else {
            double y_dif = core.cursor_pos.y - prev_pos.y;
            core.scrollbar.position += y_dif / (core.scrollbar.length - core.scrollbar.bar.size.y);

            core.scrollbar.update_pos(1.0 - core.scrollbar.position, true);

            core.start_loc = (1.0 - core.scrollbar.position) * std::max(core.rows_total - core.rows_shown, 0);
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
            bool button_pressed = false;

            for(int i : core.active_buttons) {
                if(core.buttons[i].box.contains(core.cursor_pos)) {
                    button_pressed = true;
                    switch(i) {
                        case 0: {
                            if(core.active_screen != EDITOR) {
                                core.change_active_screen(EDITOR);
                            }
                            break;
                        }
                        case 1: {
                            if(core.active_screen != TABS) {
                                core.change_active_screen(TABS);
                            }
                            break;
                        }
                        case 2: {
                            core.active_tab = core.tabs.size();
                            core.tabs.push_back(Tab());
                            core.tabs[core.active_tab].text.init_buffers();
                            core.tabs[core.active_tab].text.load_buffers(core.font, "New file", core.text_size);
                            if(core.active_screen == TABS && core.end_loc - core.start_loc == core.rows_shown) core.start_loc = std::max(core.rows_total - core.rows_shown, 0) + 1;

                            core.byte_rows.clear();
                            core.reset_total_rows();
                            break;
                        }
                        case 3: {
                            std::string filepath = open_dialog();
                            if(filepath.size() != 0) {
                                ++core.active_tab;
                                core.tabs.push_back(Tab());
                                core.tabs[core.active_tab].path = filepath;
                                core.tabs[core.active_tab].text.init_buffers();
                                core.tabs[core.active_tab].text.load_buffers(core.font, filepath, core.text_size);
                                core.tabs[core.active_tab].bytes = get_bytes_from_file(filepath.data());
                                core.tabs[core.active_tab].edited = false;

                                core.selected_num.y = -1;
                                core.start_loc = 0;

                                core.reset_total_rows();

                                core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);

                                core.byte_rows.clear();
                            }
                            break;
                        }
                        case 4: {
                            if(core.tabs[core.active_tab].path.size() == 0) {
                                std::string filepath = save_as_dialog();
                                if(filepath.size() != 0) {
                                    std::ofstream file;
                                    file.open(filepath, std::ios::out | std::ios::binary | std::ios::trunc);
                                    file.write((char*)core.tabs[core.active_tab].bytes.data(), core.tabs[core.active_tab].bytes.size());
                                    core.tabs[core.active_tab].edited = false;
                                    
                                    file.close();

                                    core.tabs[core.active_tab].path = filepath;
                                    core.tabs[core.active_tab].text.load_buffers(core.font, filepath, core.text_size);
                                }
                            } else {
                                std::ofstream file;
                                file.open(core.tabs[core.active_tab].path, std::ios::out | std::ios::binary | std::ios::trunc);
                                file.write((char*)core.tabs[core.active_tab].bytes.data(), core.tabs[core.active_tab].bytes.size());
                                core.tabs[core.active_tab].edited = false;

                                file.close();
                            }
                            break;
                        }
                        case 5: {
                            std::string filepath = save_as_dialog();
                            if(filepath.size() != 0) {
                                std::ofstream file;
                                file.open(filepath, std::ios::out | std::ios::binary | std::ios::trunc);
                                file.write((char*)core.tabs[core.active_tab].bytes.data(), core.tabs[core.active_tab].bytes.size());
                                core.tabs[core.active_tab].edited = false;

                                file.close();

                                core.tabs[core.active_tab].path = filepath;
                                core.tabs[core.active_tab].text.load_buffers(core.font, filepath, core.text_size);
                            }
                            break;
                        }
                        case 6: {
                            if(core.active_screen != SETTINGS) {
                                core.change_active_screen(SETTINGS);
                            }
                            break;
                        }
                        case 7: {
                            if(core.tabs[core.active_tab].path.size() == 0) {
                                std::string filepath = save_as_dialog();
                                if(filepath.size() != 0) {
                                    std::ofstream file;
                                    file.open(filepath, std::ios::out | std::ios::binary | std::ios::trunc);
                                    file.write((char*)core.tabs[core.active_tab].bytes.data(), core.tabs[core.active_tab].bytes.size());
                                    core.tabs[core.active_tab].edited = false;
                                    
                                    file.close();

                                    core.tabs[core.active_tab].path = filepath;
                                    core.tabs[core.active_tab].text.load_buffers(core.font, filepath, core.text_size);
                                }
                            } else {
                                std::ofstream file;
                                file.open(core.tabs[core.active_tab].path, std::ios::out | std::ios::binary | std::ios::trunc);
                                file.write((char*)core.tabs[core.active_tab].bytes.data(), core.tabs[core.active_tab].bytes.size());
                                core.tabs[core.active_tab].edited = false;

                                file.close();
                            }

                            core.tabs.erase(core.tabs.begin() + core.tab_delete);
                            core.reset_total_rows();
                            if(core.tab_delete == core.active_tab) {
                                core.active_tab = 0;
                                core.byte_rows.clear();
                                core.selected_num = {0, -1};

                                core.scrollbar.update_pos(0.0, true);
                                core.start_loc = 0;
                            } else if(core.tab_delete < core.active_tab) --core.active_tab;

                            core.change_active_screen(TABS);
                            break;
                        }
                        case 8: {
                            core.tabs.erase(core.tabs.begin() + core.tab_delete);
                            core.reset_total_rows();
                            if(core.tab_delete == core.active_tab) {
                                core.active_tab = 0;
                                core.byte_rows.clear();
                                core.selected_num = {0, -1};

                                core.scrollbar.update_pos(0.0, true);
                                core.start_loc = 0;
                            } else if(core.tab_delete < core.active_tab) --core.active_tab;
                            
                            core.change_active_screen(TABS);
                            break;
                        }
                        case 9: {
                            core.change_active_screen(TABS);
                            break;
                        }
                        case 10: {
                            core.byte_rows.clear();
                            core.text_mode = !core.text_mode;
                            
                            if(core.text_mode) {
                                core.buttons[10].text.load_buffers(core.font, "Char", core.text_size);
                            } else {
                                core.buttons[10].text.load_buffers(core.font, "Hex", core.text_size);
                            }

                            if(core.edit_color) {
                                core.edit_color = false;

                                int add_zeroes = 6 - core.color_string.size();
                                for(int i = 0; i < add_zeroes; ++i) {
                                    core.color_string += '0';
                                }

                                core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
                                if(core.color_string.size() >= 6) {
                                    int color = std::stoi(core.color_string, 0, 16);
                                    core.color = {float((color >> 16) & 0xFF) / 255, float((color >> 8) & 0xFF) / 255, float(color & 0xFF) / 255, 1.0f};
                                    if(core.selected_num.y * 16 + core.selected_num.x < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                                    core.edit_color = false;
                                }
                            }
                            break;
                        }
                        case 11: {
                            if(!core.edit_color) {
                                core.edit_color = true;
                                core.color_string = "";
                                core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
                            } else {
                                core.edit_color = false;

                                int add_zeroes = 6 - core.color_string.size();
                                for(int i = 0; i < add_zeroes; ++i) {
                                    core.color_string += '0';
                                }

                                core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
                                if(core.color_string.size() >= 6) {
                                    int color = std::stoi(core.color_string, 0, 16);
                                    core.color = {float((color >> 16) & 0xFF) / 255, float((color >> 8) & 0xFF) / 255, float(color & 0xFF) / 255, 1.0f};
                                    if(core.selected_num.y * 16 + core.selected_num.x < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                                    core.edit_color = false;
                                }
                            }
                            break;
                        }
                        case 12: {
                            core.big_endian = !core.big_endian;
                            if(core.big_endian) {
                               core.buttons[12].text.load_buffers(core.font, "Big", core.text_size);
                            } else {
                                core.buttons[12].text.load_buffers(core.font, "Little", core.text_size);
                            }
                            int loc = core.selected_num.x + core.selected_num.y * 16;
                            if(loc >= 0 && loc < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                        }
                    }
                }
            }

            if(!button_pressed) {
                if(core.active_screen == EDITOR) {
                    if(core.rows_total > core.rows_shown && core.scrollbar.bar.contains(core.cursor_pos)) {
                        core.scrollbar.clicked_on = true;
                    } else {
                        if(core.hovered_num.y >= core.start_loc && core.hovered_num.y <= core.end_loc && core.hovered_num.x >= 0 && core.hovered_num.x < 16) {
                            int loc = core.hovered_num.x + core.hovered_num.y * 16;
                            if(loc <= core.tabs[core.active_tab].bytes.size()) {
                                if(core.input_status != -1) {
                                    core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                                }

                                if(core.selected_num != core.hovered_num) {
                                    core.selected_num = core.hovered_num;
                                    if(loc != core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                                } else {
                                    core.selected_num.y = -1;
                                }

                                core.input_status = -1;
                            }
                        } else if(core.selected_num.y != -1) {
                            core.byte_rows[core.selected_num.y].load_buffers(core.text_mode, core.font, core.text_size, core.selected_num.y * 0x10, core.tabs[core.active_tab].bytes, core.ten);
                            core.selected_num.y = -1;
                            core.input_status = -1;
                        }
                    }
                } else if(core.active_screen == TABS) {
                    if(core.rows_total > core.rows_shown && core.scrollbar.bar.contains(core.cursor_pos)) {
                        core.scrollbar.clicked_on = true;
                    } else {
                        if(core.hovered_num.y >= core.start_loc && core.hovered_num.y < core.end_loc && core.cursor_pos.x >= core.scrollbar.bar.size.x) {
                            if(core.cursor_pos.x >= core.scrollbar.bar.size.x + 17 * core.text_size) {
                                if(core.active_tab == core.hovered_num.y) {
                                    core.change_active_screen(EDITOR);
                                } else {
                                    core.byte_rows.clear();
                                    core.active_tab = core.hovered_num.y;
                                    int loc = core.tabs[core.active_tab].selected.y * 16 + core.tabs[core.active_tab].selected.x;
                                    if(loc >= 0 && loc < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.tabs[core.active_tab].selected);
                                }
                            } else if(core.tabs.size() > 1) {
                                if(!core.tabs[core.hovered_num.y].edited) {
                                    core.tabs.erase(core.tabs.begin() + core.hovered_num.y);
                                    core.reset_total_rows();

                                    if(core.hovered_num.y == core.active_tab) {
                                        core.active_tab = 0;
                                        core.byte_rows.clear();
                                        core.selected_num = {0, -1};

                                        core.scrollbar.update_pos(0.0, true);
                                        core.start_loc = 0;
                                    } else if(core.tab_delete < core.active_tab) --core.active_tab;
                                } else {
                                    core.tab_delete = core.hovered_num.y;
                                    core.change_active_screen(DELETE_TAB);
                                }
                            }
                        }
                    }
                } else if(core.active_screen == SETTINGS) {
                    if(core.edit_color) {
                        core.edit_color = false;

                        int add_zeroes = 6 - core.color_string.size();
                        for(int i = 0; i < add_zeroes; ++i) {
                            core.color_string += '0';
                        }

                        core.buttons[11].text.load_buffers(core.font, core.color_string, core.text_size);
                        if(core.color_string.size() >= 6) {
                            int color = std::stoi(core.color_string, 0, 16);
                            core.color = {float((color >> 16) & 0xFF) / 255, float((color >> 8) & 0xFF) / 255, float(color & 0xFF) / 255, 1.0f};
                            if(core.selected_num.y * 16 + core.selected_num.x < core.tabs[core.active_tab].bytes.size()) core.load_info_buffer(core.selected_num);
                            core.edit_color = false;
                        }
                    }
                }
            }
        } else if(action == GLFW_RELEASE) {
            core.scrollbar.clicked_on = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);

    int new_loc = core.start_loc - (yoffset * 4);

    int max = std::max(core.rows_total - core.rows_shown, 0);

    if(new_loc < 0) {
        core.start_loc = 0;
    } else if(yoffset < 0 && new_loc > max) {
        if(core.start_loc <= max) {
            core.start_loc = max;
        }
    } else {
        core.start_loc -= (yoffset * 4);
    }

    core.scrollbar.update_pos(double(core.start_loc) / std::max(core.rows_total - core.rows_shown, 1), true);

    if(core.active_screen == EDITOR) {
        core.hovered_num = {floor(double(core.cursor_pos.x - (20 + 57 * core.text_size)) / (17 * core.text_size)), -floor(double(core.cursor_pos.y - (core.screen_size.y - 45 * core.text_size)) / (15 * core.text_size))};
    } else if(core.active_screen == TABS) {
        core.hovered_num = {floor(double(core.cursor_pos.x - (20 + 57 * core.text_size)) / (17 * core.text_size)), -floor(double(core.cursor_pos.y - (core.screen_size.y - 30 * core.text_size)) / (15 * core.text_size))};
    }
    core.hovered_num.y += core.start_loc;
}

void Core::game_loop() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat3 view_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2)));
    view_matrix = glm::translate(view_matrix, glm::vec2(-viewport_size / 2));

    end_loc = std::min(start_loc + rows_shown, (int)rows_total);


    
    textures[0].bind(0);

    for(int i : visible_buttons) {
        bool active = false;

        Button& b = buttons[i];
        b.text.vertex_buf.bind();

        glm::mat3 transform_matrix = glm::translate(identity_matrix, glm::vec2(b.box.position) + glm::vec2{5 * text_size, 3 * text_size});
        
        shaders[0].use();
        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        if(std::count(active_buttons.begin(), active_buttons.end(), i)) {
            glUniform4fv(2, 1, &color[0]);
            active = true;
        } else {
            glUniform4f(2, color.r, color.g, color.b, 0.5f);
        } 
        glDrawArrays(GL_TRIANGLES, 0, b.text.vertex_buf.vertices);

        if(active && b.box.contains(cursor_pos)) {
            shaders[1].use();
            buffers[0].bind();

            transform_matrix = glm::translate(identity_matrix, glm::vec2(buttons[i].box.position));
            transform_matrix = glm::scale(transform_matrix, glm::vec2(buttons[i].box.size));
            glUniformMatrix3fv(0, 1, false, &transform_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
            glUniform4f(2, color.r, color.g, color.b, 0.5f);
            glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
        }
    }

    if(active_screen == EDITOR) {
        glm::vec2 translate_pos = glm::vec2(20, screen_size.y - 42 * text_size);

        glm::mat3 transform_matrix = glm::translate(identity_matrix, translate_pos);

        glm::mat3 num_matrix = glm::translate(identity_matrix, glm::vec2(20 + 63 * text_size, screen_size.y - 27 * text_size));

        shaders[0].use();

        buffers[1].bind();
        storage_buffers[0].bind(0);
        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &num_matrix[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, buffers[1].vertices);

        for(int i = start_loc; i < end_loc; ++i) {
            Text_row& t = byte_rows[i];

            if(!t.vertex_buf.initialized) {
                t.init_buffers();
                t.load_buffers(text_mode, font, text_size, i * 0x10, tabs[active_tab].bytes, ten);
            }

            t.vertex_buf.bind();

            glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
            glUniform4fv(2, 1, &color[0]);
            glDrawArrays(GL_TRIANGLES, 0, t.vertex_buf.vertices);

            translate_pos += glm::vec2(0, -15 * text_size);

            transform_matrix = glm::translate(identity_matrix, translate_pos);
        }

        if(selected_num.y != -1 && selected_num.y * 16 + selected_num.x != tabs[active_tab].bytes.size()) {
            shaders[2].use();
            buffers[2].bind();
            storage_buffers[1].bind(0);
            glm::mat3 info_matrix = glm::translate(identity_matrix, {20 + (font.glyph_map['0'].stride * 40 + 106) * text_size, screen_size.y - 30 * text_size});

            glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &info_matrix[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, buffers[2].vertices);
        }

        shaders[1].use();
        buffers[0].bind();

        if(selected_num.y != -1 && selected_num.y >= start_loc && selected_num.y < end_loc) {
            glm::mat3 select_matrix = glm::translate(identity_matrix, glm::vec2(20 + 57 * text_size, screen_size.y - 45 * text_size) + glm::vec2{17, -15} * glm::vec2(selected_num.x, selected_num.y - start_loc) * float(text_size));
            select_matrix = glm::scale(select_matrix, glm::vec2{17, 15} * float(text_size));

            glUniformMatrix3fv(0, 1, false, &select_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
            glUniform4f(2, color.r, color.g, color.b, 0.25f);
            glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
        }
        if(hovered_num != selected_num && hovered_num.y >= start_loc && hovered_num.y < end_loc && hovered_num.x >= 0 && hovered_num.x < 16 && hovered_num.y * 16 + hovered_num.x <= tabs[active_tab].bytes.size()) {
            glm::mat3 hover_matrix = glm::translate(identity_matrix, glm::vec2(20 + 57 * text_size, screen_size.y - 45 * text_size) + glm::vec2{17, -15} * glm::vec2(hovered_num.x, hovered_num.y - start_loc) * float(text_size));
            hover_matrix = glm::scale(hover_matrix, glm::vec2{17, 15} * float(text_size));

            glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
            glUniform4f(2, color.r, color.g, color.b, 0.125f);
            glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
        }

        glm::mat3 scrollbar_matrix = glm::scale(identity_matrix, {10, scrollbar.length});

        glUniformMatrix3fv(0, 1, false, &scrollbar_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
        glUniform4f(2, color.r, color.g, color.b, 0.25f);
        glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);

        if(rows_total > rows_shown) {
            scrollbar_matrix = glm::translate(identity_matrix, glm::vec2(scrollbar.bar.position));
            scrollbar_matrix = glm::scale(scrollbar_matrix, glm::vec2(scrollbar.bar.size));
            glUniformMatrix3fv(0, 1, false, &scrollbar_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
            glUniform4f(2, color.r, color.g, color.b, 0.5f);
            glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
        }
    } else if(active_screen == TABS) {
        shaders[0].use();

        glm::vec2 translate_pos = {scrollbar.bar.size.x + 20 * text_size, screen_size.y - 27 * text_size};
        glm::mat3 transform_matrix = glm::translate(identity_matrix, translate_pos);

        for(int i = start_loc; i < end_loc; ++i) {
            Tab& t = tabs[i];

            if(tabs.size() > 1) {
                if(t.edited == false) {
                    buffers[4].bind();
                    transform_matrix = glm::translate(identity_matrix, {scrollbar.bar.size.x + 6 * text_size, translate_pos.y + text_size});
                } else {
                    buffers[5].bind();
                    transform_matrix = glm::translate(identity_matrix, {scrollbar.bar.size.x + 6 * text_size, translate_pos.y});
                }

                glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
                glUniform4fv(2, 1, &color[0]);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            
            transform_matrix = glm::translate(identity_matrix, translate_pos);

            t.text.vertex_buf.bind();

            glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
            glUniform4fv(2, 1, &color[0]);
            glDrawArrays(GL_TRIANGLES, 0, t.text.vertex_buf.vertices);

            translate_pos += glm::vec2(0, -15 * text_size);
        }

        shaders[1].use();
        buffers[0].bind();

        if(active_tab >= start_loc && active_tab < end_loc) {
            glm::mat3 hover_matrix = glm::translate(identity_matrix, glm::vec2(scrollbar.bar.size.x + 17 * text_size, (screen_size.y - 30 * text_size) - 15 * (active_tab - start_loc) * text_size));
            hover_matrix = glm::scale(hover_matrix, glm::vec2{screen_size.x - (scrollbar.bar.size.x + 17 * text_size), 15 * text_size});

            glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
            glUniform4f(2, color.r, color.g, color.b, 0.25f);
            glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
        }

        if(!scrollbar.clicked_on && hovered_num.y >= start_loc && hovered_num.y < end_loc && cursor_pos.x >= scrollbar.bar.size.x) {
            if(cursor_pos.x < scrollbar.bar.size.x + 17 * text_size) {
                if(tabs.size() > 1) {
                    glm::mat3 hover_matrix = glm::translate(identity_matrix, glm::vec2(scrollbar.bar.size.x, (screen_size.y - 30 * text_size) - 15 * (hovered_num.y - start_loc) * text_size));
                    hover_matrix = glm::scale(hover_matrix, glm::vec2{17 * text_size, 15 * text_size});
                    glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
                    glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
                    glUniform4f(2, color.r, color.g, color.b, 0.125f);
                    glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
                }
            } else if(hovered_num.y != active_tab) {
                glm::mat3 hover_matrix = glm::translate(identity_matrix, glm::vec2(scrollbar.bar.size.x + 17 * text_size, (screen_size.y - 30 * text_size) - 15 * (hovered_num.y - start_loc) * text_size));
                hover_matrix = glm::scale(hover_matrix, glm::vec2{screen_size.x - (scrollbar.bar.size.x  + 17 * text_size), 15 * text_size});
                glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
                glUniform4f(2, color.r, color.g, color.b, 0.125f);
                glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
            }
        }

        glm::mat3 scrollbar_matrix = glm::scale(identity_matrix, {10, scrollbar.length});
        glUniformMatrix3fv(0, 1, false, &scrollbar_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
        glUniform4f(2, color.r, color.g, color.b, 0.25f);
        glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);

        if(rows_total > rows_shown) {
            scrollbar_matrix = glm::translate(identity_matrix, glm::vec2(scrollbar.bar.position));
            scrollbar_matrix = glm::scale(scrollbar_matrix, glm::vec2(scrollbar.bar.size));
            glUniformMatrix3fv(0, 1, false, &scrollbar_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
            glUniform4f(2, color.r, color.g, color.b, 0.5f);
            glDrawArrays(GL_TRIANGLES, 0, buffers[0].vertices);
        }
    } else if(active_screen == DELETE_TAB) {
        shaders[0].use();
        buffers[3].bind();

        glm::mat3 transform_matrix = glm::translate(identity_matrix, glm::vec2{5 * text_size, screen_size.y - 27 * text_size});

        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        glUniform4fv(2, 1, &color[0]);
        glDrawArrays(GL_TRIANGLES, 0, buffers[3].vertices);
    } else if(active_screen == SETTINGS) {
        shaders[0].use();

        texts[0].vertex_buf.bind();
        glm::mat3 transform_matrix = glm::translate(identity_matrix, glm::vec2{5 * text_size, screen_size.y - 27 * text_size});
        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        glUniform4fv(2, 1, &color[0]);
        glDrawArrays(GL_TRIANGLES, 0, texts[0].vertex_buf.vertices);

        texts[1].vertex_buf.bind();
        transform_matrix = glm::translate(transform_matrix, glm::vec2{0, -15 * text_size});
        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        glUniform4fv(2, 1, &color[0]);
        glDrawArrays(GL_TRIANGLES, 0, texts[1].vertex_buf.vertices);

        texts[2].vertex_buf.bind();
        transform_matrix = glm::translate(transform_matrix, glm::vec2{0, -15 * text_size});
        glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &transform_matrix[0][0]);
        glUniform4fv(2, 1, &color[0]);
        glDrawArrays(GL_TRIANGLES, 0, texts[2].vertex_buf.vertices);
    }

    glm::mat3 cursor_matrix = glm::translate(identity_matrix, glm::vec2(cursor_pos.x, cursor_pos.y - 32));
    cursor_matrix = glm::scale(cursor_matrix, {32, 32});

    shaders[0].use();
    textures[1].bind(0);
    buffers[6].bind();

    glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &cursor_matrix[0][0]);
    glUniform4fv(2, 1, &color[0]);
    glDrawArrays(GL_TRIANGLES, 0, buffers[6].vertices);


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
    core.window = glfwCreateWindow(core.screen_size.x, core.screen_size.y, "Avery's Hex Editor", NULL, NULL);
    if(core.window == NULL) {
        std::cout << "ERROR: Window creation failed.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(core.window);

    glfwSetInputMode(core.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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