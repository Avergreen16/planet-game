#pragma once
#include "func.cpp"

using any_asset = std::variant<Buffer>;

struct Core;

struct Button {
    std::vector<any_asset> assets;
    AABB box;
    std::function<void(Button&)> calc_func;
    std::function<void(Button&)> draw_func;

    Button(std::function<void(Button&)> calc_func, std::function<void(Button&)> draw_func) {
        this->calc_func = calc_func;
        this->draw_func = draw_func;
    }

    /*Button(const Button& b) {
        box = b.box;
        on_click = b.on_click;
        on_draw = b.on_draw;
        on_hover = b.on_hover;
    }

    Button& operator=(const Button& b) {
        box = b.box;
        on_click = b.on_click;
        on_draw = b.on_draw;
        on_hover = b.on_hover;

        return *this;
    }*/

    Button(Button&& b) noexcept {
        box = b.box;
        std::swap(calc_func, b.calc_func);
        std::swap(draw_func, b.draw_func);
    }

    Button& operator=(Button&& b) {
        box = b.box;
        std::swap(calc_func, b.calc_func);
        std::swap(draw_func, b.draw_func);

        return *this;
    }

    inline void calc() {
        calc_func(*this);
    }
    inline void draw() {
        draw_func(*this);
    }

    ~Button() noexcept = default;
};

struct Text {
    int size = 1;
    glm::vec4 color;
    AABB box = {{0, 0}, {0, 0}};
    Buffer vertex_buf;

    Text(int s, glm::vec4 v) {
        size = s;
        color = v;
    }
    Text() = default;

    Text(Text&& a) noexcept {
        size = a.size;
        color = a.color;
        box = a.box;
        std::swap(vertex_buf, a.vertex_buf);
    }
    Text& operator=(Text&& a) = default;

    void init_buffers() {
        vertex_buf.init();
    }

    void load_buffers(Font_data& font, std::string text) {
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

        vertex_buf.bind();
        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(Vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        box.size = glm::ivec2{std::max(0, pos - 1 * size), font.line_height * size};
    }

    void draw();

    ~Text() noexcept = default;
};

struct Text_box {
    std::vector<any_asset> assets;
    AABB box;
    Text text;
    std::string str;
    int pos = 0;
    glm::ivec2 pos_coord = {0, 0};
    bool hovered = false;
    bool selected = false;
    
    std::function<void(Text_box&)> calc_func;
    std::function<void(Text_box&)> draw_func;

    Text_box(std::function<void(Text_box&)> calc_func, std::function<void(Text_box&)> draw_func, AABB init_box, glm::ivec2 text_offset, int size, glm::vec4 color) {
        text.size = size;
        text.color = color;
        text.box.position = init_box.position + text_offset;
        text.init_buffers();

        this->box = init_box;

        this->calc_func = calc_func;
        this->draw_func = draw_func;
    }

    Text_box(Text_box&& b) noexcept {
        str = b.str;
        box = b.box;
        std::swap(text, b.text);
        std::swap(calc_func, b.calc_func);
        std::swap(draw_func, b.draw_func);
    }

    Text_box& operator=(Text_box&& b) noexcept {
        str = b.str;
        box = b.box;
        std::swap(text, b.text);
        std::swap(calc_func, b.calc_func);
        std::swap(draw_func, b.draw_func);

        return *this;
    }

    inline void update_text();

    void input(uint8_t c) {
        str.insert(str.begin() + pos, c);
        ++pos;

        update_text();
    }

    void backspace() {
        if(pos != 0) str.erase(str.begin() + pos - 1);
        --pos;

        update_text();
    }

    inline void calc() {
        calc_func(*this);
    }
    inline void draw() {
        draw_func(*this);
    }

    ~Text_box() noexcept = default;
};

using Widget = std::variant<Button, Text, Text_box>;