#pragma once
#include "func.cpp"

using any_asset = std::variant<Buffer>;

struct Core;

struct Button {
    std::vector<any_asset> assets;
    AABB hitbox = {{6, 7}, {8, 9}};
    std::function<void(Button&)> on_click;
    std::function<void(Button&)> on_draw;

    Button(std::function<void(Button&)> click_func, std::function<void(Button&)> draw_func) {
        on_draw = draw_func;
        on_click = click_func;
    }

    Button(const Button& b) {
        hitbox = b.hitbox;
        on_click = b.on_click;
        on_draw = b.on_draw;
    }

    Button& operator=(const Button& b) {
        hitbox = b.hitbox;
        on_click = b.on_click;
        on_draw = b.on_draw;

        return *this;
    }

    Button(Button&& b) {
        hitbox = b.hitbox;
        std::swap(on_click, b.on_click);
        std::swap(on_draw, b.on_draw);
    }

    Button& operator=(Button&& b) {
        hitbox = b.hitbox;
        std::swap(on_click, b.on_click);
        std::swap(on_draw, b.on_draw);

        return *this;
    }

    inline void draw() {
        on_draw(*this);
    }
    inline void click() {
        on_click(*this);
    }
};

struct Text {
    Buffer vertex_buf;
    AABB box = {{0, 0}, {0, 0}};
    glm::vec4 color;

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

        box.size = glm::ivec2{std::max(0, pos - 1 * size), font.line_height * size};
    }

    void draw();
};

using any_widget = std::variant<Button, Text>;