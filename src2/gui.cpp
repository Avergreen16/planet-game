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

using any_widget = std::variant<Button>;