#pragma once
#include "stuff.cpp"
#include "gui\gui.cpp"

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};
    //glm::ivec2 monitor_size = {0, 0};
    glm::ivec2 cursor_pos = {0, 0};

    glm::mat3 screen_matrix;

    double scale = 1;
    glm::dvec2 camera_pos = {0, 0};

    std::vector<Planetoid> planetoids;

    Buffer quadtree_buffer;
    Shader flat_shader;

    Buffer circle_buffer;
    Shader shader_circle;

    Buffer x_buffer;

    uint64_t time;

    Gui_core gui_core;
    int num_planetoids = -1;
    int selected_planetoid = -1;
    double selected_mass = -1;

    std::unordered_map<GLuint, bool> key_map = {
        {GLFW_KEY_RIGHT_SHIFT, false}
    };

    std::unordered_map<GLuint, bool> mouse_button_map = {
        {GLFW_MOUSE_BUTTON_LEFT, false},
        {GLFW_MOUSE_BUTTON_RIGHT, false}
    };

    void init() {
        screen_matrix = glm::translate(glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2))), glm::vec2(-viewport_size / 2));

        circle_buffer.init();

        std::vector<glm::vec2> vertices = {
            {-1, -1},
            {1, -1},
            {-1, 1},
            {-1, 1},
            {1, -1},
            {1, 1}
        };

        circle_buffer.set_data(vertices.data(), 6, sizeof(float) * 2);
        circle_buffer.set_attrib(0, 2, sizeof(float) * 2, 0);

        quadtree_buffer.init();

        shader_circle.compile("res/shaders/circle.vs", "res/shaders/circle.fs");
        flat_shader.compile("res/shaders/select.vs", "res/shaders/select.fs");

        glm::mat3 rot_matrix = glm::rotate(identity_matrix, float(0.5 * M_PI));
        planetoids.push_back(Planetoid{glm::vec4{1.0, 0x0.8p0, 0.0, 1.0}, glm::dvec2{0, 0}, cbrt(0x400), 0x400});

        std::array<glm::vec2, 4> x_vertices = {
            glm::vec2{-2, -2},
            glm::vec2{3, 3},
            glm::vec2{3, -2},
            glm::vec2{-2, 3}
        };

        x_buffer.init();
        x_buffer.set_data(x_vertices.data(), 4, sizeof(float) * 2);
        x_buffer.set_attrib(0, 2, sizeof(float) * 2, 0);

        for(int i = 0; i < 0x400; ++i) {
            int random = rand();
            double mass = float(random) / 32768 * 0x0.Fp0 + 0x0.1p0;
            glm::vec2 pos = {gaussian_random(0, 0x400), gaussian_random(0, 0x400)};

            planetoids.push_back(Planetoid{glm::vec4{0x0.2p0 + float(rand()) / 32768 * 0x0.3p0, 0x0.2p0 + float(rand()) / 32768 * 0x0.3p0, 0x0.2p0 + float(rand()) / 32768 * 0x0.3p0, 1.0f}, pos, cbrt(mass), mass, glm::dvec2((rot_matrix * glm::vec3(glm::normalize(pos), 1.0)).xy()) * (sqrt(G * 0x400 / glm::length(pos)))});
        }

        /*for(int i = 0; i < stars_in_core; ++i) {
            int random = rand();
            double mass = 0x200 + (double(random) / 32768) * 0x7F00;
            glm::vec2 pos = {gaussian_random(0, core_x_dist), gaussian_random(0, core_y_dist)};

            float random_col = float(random) / 32768 * 1.5f;
            planetoids.push_back(Planetoid{glm::vec4{1.0f, std::min(random_col, 1.0f), std::max(random_col - 1.0f, 0.0f), 1.0f}, pos, cbrt(mass), mass, glm::dvec2((rot_matrix * glm::vec3(glm::normalize(pos), 1.0)).xy()) * (sqrt(G * planetoids[0].mass / glm::length(pos)))});
        }

        for(int a = 0; a < arms; ++a) {
            for(int i = 0; i < stars_per_arm; ++i) {
                int random = rand();
                double mass = 0x200 + (double(random) / 32768) * 0x7F00;
                glm::vec2 pos = spiral_func({gaussian_random(arm_x_mean + arm_x_offset, arm_x_dist), gaussian_random(arm_y_mean, arm_y_dist)}, a * 2 * M_PI / arms);

                float random_col = float(random) / 32768 * 1.5f;
                planetoids.push_back(Planetoid{glm::vec4{1.0f, std::min(random_col, 1.0f), std::max(random_col - 1.0f, 0.0f), 1.0f}, pos, cbrt(mass), mass, glm::dvec2((rot_matrix * glm::vec3(glm::normalize(pos), 1.0)).xy()) * (sqrt(G * planetoids[0].mass / glm::length(pos)))});
            }
        }*/

        gui_core.init();
    }

    void char_update(unsigned int codepoint) {
        if((codepoint & 0xFF) == codepoint) gui_core.events.push_back(Char_event{(uint8_t)codepoint});
    }

    void framebuffer_update(int width, int height) {
        screen_size.x = width;
        screen_size.y = height;
        viewport_size.x = width + 1 * (width & 1);
        viewport_size.y = height + 1 * (height & 1);

        glViewport(0, 0, viewport_size.x, viewport_size.y);

        screen_matrix = glm::translate(glm::scale(identity_matrix, glm::vec2(2.0 / viewport_size.x, 2.0 / viewport_size.y)), glm::vec2(-viewport_size / 2));
        
        game_loop();
    }

    void cursor_pos_update(double xpos, double ypos) {
        glm::ivec2 new_cursor_pos = {xpos, screen_size.y - ypos - 1};

        if(mouse_button_map[GLFW_MOUSE_BUTTON_RIGHT]) {
            glm::dvec2 difference = new_cursor_pos - cursor_pos;

            camera_pos -= difference / (16 * scale);
        }

        cursor_pos = new_cursor_pos;
    }

    void mouse_button_update(int button, int action) {
        gui_core.events.push_back(Mouse_button_event{button, action});
        if(mouse_button_map.contains(button)) {
            if(action == GLFW_PRESS) mouse_button_map[button] = true;
            else if(action == GLFW_RELEASE) mouse_button_map[button] = false;
        }

        if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            glm::mat3 matrix = glm::translate(glm::scale(glm::translate(identity_matrix, glm::vec2(camera_pos)), {1.0 / (16 * scale), 1.0 / (16 * scale)}), glm::vec2(-screen_size / 2));
            glm::vec2 cursor_world_pos = (matrix * glm::vec3(cursor_pos, 1.0)).xy();

            bool deselect = true;
            for(int i = 0; i < planetoids.size(); i++) {
                glm::vec2 diff = glm::vec2(planetoids[i].position) - cursor_world_pos;
                if(diff.x * diff.x + diff.y * diff.y < planetoids[i].radius * planetoids[i].radius) {
                    deselect = false;
                    selected_planetoid = i;
                    break;
                }
            }
            if(deselect) selected_planetoid = -1;
        }
    }

    void key_update(int key, int action) {
        gui_core.events.push_back(Key_event{key, action});

        if(key_map.contains(key)) {
            if(action == GLFW_PRESS) key_map[key] = true;
            else if(action == GLFW_RELEASE) key_map[key] = false;
        }

        if(key == GLFW_KEY_MINUS) {
            if(action == GLFW_PRESS) {
                simulation_speed /= 2;
            }
        } else if(key == GLFW_KEY_EQUAL) {
            if(action == GLFW_PRESS) {
                simulation_speed *= 2;
            }
        } else if(key == GLFW_KEY_SPACE) {
            if(action == GLFW_PRESS) {
                sim_paused = !sim_paused;
            }
        }
    }

    void scroll_update(double yoffset) {
        if(yoffset > 0) {
            scale *= 1.2;
        } else if(yoffset < 0) {
            scale /= 1.2;
        }
    }

    void game_loop();
};

Core core;

#include "gui\parameter_functions.cpp"

void init_widgets(Gui_core& self) {
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, 0xC}});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x1E}});
    std::get<1>(core.gui_core.widgets[1]).load_buffers(self.font, "Hello world!");
    //core.gui_core.widgets.push_back(std::move(Text_box(tb0_calc, tb0_draw, {{16, 34}, {64, 64}}, {8, 8}, 2, {0.0, 1.0, 0.0, 1.0})));
    //core.gui_core.widgets.push_back(std::move(Text{2, {0.0, 1.0, 0.0, 1.0}}));
    //std::get<1>(core.gui_core.widgets[1]).box.position = {128, 128};
    //std::get<1>(core.gui_core.widgets[1]).init_buffers();
    //std::get<1>(core.gui_core.widgets[1]).load_buffers(core.gui_core.font, "Hello world!");
}