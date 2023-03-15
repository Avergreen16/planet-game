#include "wrapper.cpp"
#include "gui.cpp"

enum buffer_id{b_rect, NUM_BUFFERS};
enum shader_id{s_text_col, s_flat, NUM_SHADERS};
enum texture_id{t_text, NUM_TEXTURES};

struct Key_event {
    int key;
    int action;
};

struct Mouse_button_event {
    int button;
    int action;
};

struct Char_event {
    uint8_t c;
};

using Event = std::variant<Key_event, Mouse_button_event, Char_event>;

/*template<typename type>
struct vec {
    type* data = 0;
    std::size_t size = 0;
    std::size_t capacity = 1;

    void resize(std::size_t new_size) {
        std::size_t new_capacity = 1;
        while(new_capacity < new_size) {
            new_capacity *= 2;
        }
        type* new_data = new type[new_capacity];
        for(int i = 0; i < std::min(size, new_size); i++) {
            new_data[i] = std::move(data);
        }
        delete[] data;
        data = new_data;
        size = std::min(size, new_size);
        capacity = new_capacity;
    }

    void push(std::initializer_list<type>& a) {
        if(a.size() + size >= capacity) {
            capacity *= 2;
            resize(capacity);
        }
        for(int i = 0; i < a.size(); i++) {
            data[size] = std::move[a.begin() + i];
            size++;
        }
    }

    type& operator[](uint32_t i) {
        return data[i];
    }

    vec() = default;

    ~vec() {
        delete[] data;
    }

    vec(std::initializer_list<type>& a) {
        push(a);
    }

    vec& operator=(std::initializer_list<type>&& a) {
        push(a);
        return *this;
    }

    vec(vec&& a) {
        std::swap(data, a);
        std::swap(size, a.size);
        std::swap(capacity, a.capacity);
    }

    vec& operator=(vec&& a) {
        std::swap(data, a);
        std::swap(size, a.size);
        std::swap(capacity, a.capacity);

        return *this;
    }

    vec(vec& a) {
        data = new type[a.capacity];
        for(int i = 0; i < a.size; i++) {
            data[i] = a.data[i];
        }
        size = a.size;
        capacity = a.capacity;
    }

    vec& operator=(const vec& a) {
        delete[] data;
        data = new type[a.capacity];
        for(int i = 0; i < a.size; i++) {
            data[i] = a.data[i];
        }
        size = a.size;
        capacity = a.capacity;

        return *this;
    }
};*/

struct Gui_core {
    Font_data font = Font_data("res/text_data.bin");

    std::vector<Buffer> buffers;
    std::vector<Shader> shaders;
    std::vector<Texture> textures;

    std::vector<Widget> widgets;

    std::vector<Event> events;

    void init();

    void calc();

    void draw();
};

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