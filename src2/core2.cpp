#include "wrapper.cpp"
#include "gui.cpp"

enum shader_id{_text_col, _NUM_SHADERS};
enum texture_id{_text, _NUM_TEXTURES};

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};

    glm::mat3 view_matrix;

    Font_data font = Font_data("res/text_data.bin");
    std::vector<Shader> shaders;
    std::vector<Texture> textures;
    std::vector<any_widget> widgets;

    void init();
    void game_loop();
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    core.screen_size.x = width;
    core.screen_size.y = height;
    core.viewport_size.x = width + 1 * (width & 1);
    core.viewport_size.y = height + 1 * (height & 1);

    glViewport(0, 0, core.viewport_size.x, core.viewport_size.y);

    core.view_matrix = glm::translate(glm::inverse(glm::scale(identity_matrix, glm::vec2(core.viewport_size / 2))), glm::vec2(-core.viewport_size / 2));

    core.game_loop();
}

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
}

Core core;