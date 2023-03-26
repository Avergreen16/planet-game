#pragma once
#include "..\wrapper.cpp"

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

std::string integers = "0123456789\x80\x81\x82\x83\x84\x85";

std::string to_base(int num, int base) {
    std::string ret;

    while(num > 0) {
        ret += integers[num % base];
        num /= base;
    }

    if(ret.size() == 0) ret = "0";

    if(num < 0) ret += '-';
    
    std::reverse(ret.begin(), ret.end());

    return ret;
}

int from_base(std::string num, int base) {
    int ret = 0;

    bool neg = false;
    if(num[0] == '-') {
        neg = true;
        num.erase(0);
    }

    for(uint8_t c : num) {
        ret *= base;
        if(c >= '0' && c <= '9') ret += c - '0';
        else if(c >= 0x80 && c <= 0x85) ret += c - 0x76;
    }

    if(neg) ret *= -1;

    return ret;
}