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

    void init_buffers() {
        vertex_buf.init();
    }

    void load_buffers(Font_data& font, std::string text) {
        int pos = 0;

        std::vector<vertex> vertices;
        std::vector<glm::vec4> colors;

        for(char c : text) {
            if(font.glyph_map.contains(c)) {
                glyph_data& g = font.glyph_map[c];

                pos += g.advance1 * size;
                
                if(g.visible) {
                    insert_char(vertices, font, size, g, {pos, 0});
                }
                pos += (g.tex_width + g.advance2) * size;
            }
        }

        vertex_buf.bind();
        vertex_buf.set_data(vertices.data(), vertices.size(), sizeof(vertex));
        vertex_buf.set_attrib(0, 2, sizeof(float) * 4, 0);
        vertex_buf.set_attrib(1, 2, sizeof(float) * 4, sizeof(float) * 2);

        box.size = glm::ivec2{std::max(0, pos - 1 * size), font.line_height * size};
    }

    Text(int s, glm::vec4 c, glm::vec2 pos) {
        size = s;
        color = c;
        box.position = pos;

        init_buffers();
    }

    Text(int s, glm::vec4 c, glm::vec2 pos, Font_data& font, std::string str) {
        size = s;
        color = c;
        box.position = pos;

        init_buffers();
        load_buffers(font, str);
    }

    Text() = default;

    Text(Text&& a) noexcept {
        size = a.size;
        color = a.color;
        box = a.box;
        std::swap(vertex_buf, a.vertex_buf);
    }
    Text& operator=(Text&& a) = default;

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

enum buffer_id{b_rect, NUM_BUFFERS};
enum shader_id{s_text_col, s_flat, NUM_SHADERS};
enum texture_id{t_text, NUM_TEXTURES};

struct key_event {
    int key;
    int action;
};

struct mouse_button_event {
    int button;
    int action;
};

struct char_event {
    uint8_t c;
};

using event = std::variant<key_event, mouse_button_event, char_event>;

struct Gui_core {
    Font_data font = Font_data("res/text_data.bin");

    std::vector<Buffer> buffers;
    std::vector<Shader> shaders;
    std::vector<Texture> textures;

    std::vector<Widget> widgets;

    std::vector<event> events;

    void init();

    void calc();

    void draw();
};

void init_widgets(Gui_core& self);

void Gui_core::init() {
    init_widgets(*this);

    buffers = std::vector<Buffer>{NUM_BUFFERS};

    std::vector<glm::vec2> vertices = {
        glm::vec2{0.0, 0.0},
        glm::vec2{1.0, 0.0},
        glm::vec2{0.0, 1.0},
        glm::vec2{0.0, 1.0},
        glm::vec2{1.0, 0.0},
        glm::vec2{1.0, 1.0}
    };

    buffers[b_rect].init();
    buffers[b_rect].set_data(vertices.data(), vertices.size(), sizeof(float) * 2);
    buffers[b_rect].set_attrib(0, 2, sizeof(float) * 2, 0);

    shaders = std::vector<Shader>{NUM_SHADERS};
    shaders[s_text_col].compile("res/shaders/text_color.vs", "res/shaders/text_color.fs");
    shaders[s_flat].compile("res/shaders/select.vs", "res/shaders/select.fs");

    stbi_set_flip_vertically_on_load(true);
    textures = std::vector<Texture>{NUM_TEXTURES};
    textures[t_text].load("res/text.png");
}

void Gui_core::calc() {
    for(Widget& w : widgets) {
        switch(w.index()) {
            case 0: {
                Button& a = std::get<0>(w);
                a.calc();

                break;
            }
            case 2: {
                Text_box& a = std::get<2>(w);

                a.calc();
                break;
            }
        }
    }

    events.clear();
}

void Gui_core::draw() {
    for(int i = 0; i < widgets.size(); i++) {
        Widget& w = widgets[i];
        switch(w.index()) {
            case 0: {
                Button& a = std::get<0>(w);
                a.draw();

                break;
            }
            case 1: {
                Text& a = std::get<1>(w);
                a.draw();

                break;
            }
            case 2: {
                Text_box& a = std::get<2>(w);
                a.draw();

                break;
            }
        }
    }
}