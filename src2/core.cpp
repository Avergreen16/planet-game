#include "gui.cpp"

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};
    glm::ivec2 cursor_pos = {0, 0};

    glm::mat3 screen_matrix;

    Gui_core gui_core;

    std::unordered_map<GLuint, bool> key_map = {
        {GLFW_KEY_RIGHT_SHIFT, false}
    };

    void init();

    void game_loop();
};

Core core;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    core.screen_size.x = width;
    core.screen_size.y = height;
    core.viewport_size.x = width + 1 * (width & 1);
    core.viewport_size.y = height + 1 * (height & 1);

    glViewport(0, 0, core.viewport_size.x, core.viewport_size.y);

    core.screen_matrix = glm::translate(glm::inverse(glm::scale(identity_matrix, glm::vec2(core.viewport_size / 2))), glm::vec2(-core.viewport_size / 2));

    core.game_loop();
}

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    if((codepoint & 0xFF) == codepoint) core.gui_core.events.push_back(Char_event{(uint8_t)codepoint});
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    core.gui_core.events.push_back(Key_event{key, action});

    if(core.key_map.contains(key)) {
        if(action == GLFW_PRESS) core.key_map[key] = true;
        else if(action == GLFW_RELEASE) core.key_map[key] = false;
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    core.cursor_pos = {xpos, core.screen_size.y - ypos - 1};
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    core.gui_core.events.push_back(Mouse_button_event{button, action});
}

#include "parameter_functions.cpp"

std::vector<Widget>& operator<<(std::vector<Widget>& vec, Widget&& w) {
    vec.emplace_back(std::move(w));
    return vec;
}

void init_widgets(Gui_core& self) {
    self.widgets << Text(2, {1.0, 1.0, 0.0, 1.0}, {0xC, -0x18}, self.font, "Hello world!");
    self.widgets << Text_box(tb0_calc, tb0_draw, {{8, 0x1C}, {0xC, 0xC}}, {4, 4}, 2, {1.0, 1.0, 0.0, 1.0});
    self.widgets << Text_box(tb0_calc, tb0_draw, {{8, 0x1C}, {0xC, 0x2C}}, {4, 4}, 2, {1.0, 1.0, 0.0, 1.0});
}