#pragma once

#include "wrapper.cpp"

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

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

    Font_data(std::string filepath) {
        std::ifstream file;
        file.open(filepath, std::ios::in | std::ios::binary);

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

void insert_char(std::vector<Vertex>& vertices, Font_data& font, int size, Glyph_data& g, glm::ivec2 pos) {
    vertices.push_back({{pos.x, pos.y}, {g.tex_coord, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x, pos.y + font.line_height * size}, {g.tex_coord, font.line_height}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y}, {g.tex_coord + g.tex_width, 0}});
    vertices.push_back({{pos.x + g.tex_width * size, pos.y + font.line_height * size}, {g.tex_coord + g.tex_width, font.line_height}});
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

uint8_t to_digit(int num) {
    if(num < 10) {
        return '0' + num;
    } else if(num < 16) {
        return 0x80 + num - 0xA;
    } else {
        return 'A' + num - 0x10;
    }
}

int from_digit(uint8_t c) {
    if(c >= '0' && c <= '9') {
        return c - '0';
    } else if(c >= 0x80 && c <= 0x85) {
        return c - 0x80;
    } else if(c >= 'A') {
        return c - 'A';
    }
}

std::string to_base(int num, int base) {
    bool is_neg = num < 0;
    num = abs(num);

    std::string inverse;
    if(num == 0) {
        return "0";
    } else {
        while(num != 0) {
            inverse += to_digit(num % base);
            num /= base;
        }
    }

    std::string ret;
    if(is_neg) ret += '-';
    for(int i = inverse.size() - 1; i >= 0; --i) {
        ret += inverse[i];
    }
    return ret;
}

int to_int(std::string& str, int base) {
    int ret = 0;
    for(int i = 0; i < str.size() - 1; ++i) {
        uint8_t c = str[str.size() - 1 - i];

        if(c == '-') {
            return -ret;
        }

        ret += from_digit(c) * pow(base, i);
    }

    return ret;
}

std::string to_balanced_trinary(int num) {
    bool is_neg = num < 0;
    num = abs(num);

    std::string inverted_ret;
    if(num == 0) {
        inverted_ret = "0";
    } else {
        while(num != 0) {
            inverted_ret += std::to_string(num % 3);
            num /= 3;
        }
    }

    bool prev_carry = false;
    bool next_carry = false;
    for(int i = 0; i < inverted_ret.size(); i++) {
        uint8_t& current = *(uint8_t*)&inverted_ret[i];
        if(current == '2') {
            current = 0x86;
            next_carry = true;
        }

        if(prev_carry == true) {
            switch(current) {
                case 0x86: {
                    current = '0';
                    break;
                }
                case '0': {
                    current = '1';
                    break;
                }
                case '1': {
                    current = 0x86;
                    next_carry = true;
                    break;
                }
            }
        }
        prev_carry = next_carry;
        next_carry = false;
    }
    if(prev_carry == true) {
        inverted_ret += '1';
    }

    std::string ret;
    if(is_neg) {
        for(int i = inverted_ret.size() - 1; i >= 0; i--) {
            if(inverted_ret[i] == 0x86) ret += '1';
            else if(inverted_ret[i] == '1') ret += 0x86;
            else ret += '0';
        }
    } else { 
        for(int i = inverted_ret.size() - 1; i >= 0; i--) {
            ret += inverted_ret[i];
        }
    }

    return ret;
}

int from_balanced_trinary(std::string input) {
    int ret = 0;
    int size = input.size();
    for(int i = 0; i < input.size(); i++) {
        uint8_t c = input[i];
        if(c == 0x86) {
            ret -= std::pow(3, size - i - 1);
        } else if(c == '1') {
            ret += std::pow(3, size - i - 1);
        }
    }

    return ret;
}