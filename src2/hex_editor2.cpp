#include "gui_func.cpp"

void Gui_core::init() {
    widgets.push_back(std::move(Text_box(tb0_calc, tb0_draw, {{16, 34}, {64, 64}}, {8, 8}, 2, {0.0, 1.0, 0.0, 1.0})));
    widgets.push_back(std::move(Text{2, {0.0, 1.0, 0.0, 1.0}}));
    std::get<1>(widgets[1]).box.position = {128, 128};
    std::get<1>(widgets[1]).init_buffers();
    std::get<1>(widgets[1]).load_buffers(font, "Hello world!");

    

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

void Core::init() {
    screen_matrix = glm::inverse(glm::scale(identity_matrix, glm::vec2(viewport_size / 2)));
    screen_matrix = glm::translate(screen_matrix, glm::vec2(-viewport_size / 2));

    gui_core.init();
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

void Core::game_loop() {
    glfwPollEvents();

    gui_core.calc();
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    gui_core.draw();
    
    glfwSwapBuffers(window);

    core.game_running = !glfwWindowShouldClose(core.window);
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

    //glfwSetWindowUserPointer(core.window, &core);

    glfwSetFramebufferSizeCallback(core.window, framebuffer_size_callback);
    glfwSetCharCallback(core.window, char_callback);
    glfwSetKeyCallback(core.window, key_callback);
    glfwSetCursorPosCallback(core.window, cursor_pos_callback);
    glfwSetMouseButtonCallback(core.window, mouse_button_callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    core.init();

    while(core.game_running) {
        core.game_loop();
    }
}