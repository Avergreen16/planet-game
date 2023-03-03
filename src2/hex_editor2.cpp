#define USE_2D
#include "gui_func.cpp"

void Core::init() {
    view_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2)));
    view_matrix = glm::translate(view_matrix, glm::vec2(-viewport_size / 2));

    widgets = {Button(b0_click, b0_draw), Text()};
    std::get<1>(widgets[1]).init_buffers();
    std::get<1>(widgets[1]).load_buffers(font, "Hello world!", 2);
    std::get<1>(widgets[1]).color = {0.0, 1.0, 0.0, 1.0};
    //std::get<1>(widgets[1]).box.position = {0, 0};

    shaders = std::vector<Shader>{_NUM_SHADERS};
    shaders[_text_col].compile(get_text_from_file("res/shaders/text_color.vs").data(), get_text_from_file("res/shaders/text_color.fs").data());

    stbi_set_flip_vertically_on_load(true);
    textures = std::vector<Texture>{_NUM_TEXTURES};
    textures[_text].load("res/text.png");
}

void Core::game_loop() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for(any_widget& a : widgets) {
        switch(a.index()) {
            case 0: {
                Button& button = std::get<0>(a);
                button.draw();

                break;
            }
            case 1: {
                Text& text = std::get<1>(a);

                text.draw();
                break;
            }
        }
    }
    
    glfwSwapBuffers(window);
}

int main() {
    // init glfw and set version
    if(glfwInit() == GLFW_FALSE) {
        std::cout << "ERROR: GLFW failed to load.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window
    core.window = glfwCreateWindow(core.screen_size.x, core.screen_size.y, "This is a test.", NULL, NULL);
    if(core.window == NULL) {
        std::cout << "ERROR: Window creation failed.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(core.window);

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

    core.init();

    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}