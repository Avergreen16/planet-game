#define USE_2D
#include "wrapper.cpp"

uint64_t get_time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

int trail_lim = 0x20;
double G = 0.0001;
double simulation_speed = 1;
bool sim_paused = true;

double quadtree_rad = 0x10000;

struct Planetoid {
    glm::vec4 color;
    glm::dvec2 position = {0, 0};
    double radius = 1;
    double mass = 1;

    glm::dvec2 velocity = {0, 0};
    glm::dvec2 accel = {0, 0};

    Buffer trail_buffer;
    std::vector<glm::vec2> trail;

    Planetoid(glm::vec4 c, glm::dvec2 p, double r, double m, glm::dvec2 v = {0, 0}) {
        color = c;
        position = p;
        radius = r;
        mass = m;
        velocity = v;

        trail_buffer.init();
        trail = {glm::vec2{position}};
    }

    Planetoid(Planetoid&& a) noexcept {
        color = a.color;
        position = a.position;
        radius = a.radius;
        mass = a.mass;
        velocity = a.velocity;

        trail = std::move(a.trail);
        std::swap(trail_buffer, a.trail_buffer);
    }

    Planetoid& operator=(Planetoid&& a) noexcept {
        color = a.color;
        position = a.position;
        radius = a.radius;
        mass = a.mass;
        velocity = a.velocity;

        trail = std::move(a.trail);
        std::swap(trail_buffer, a.trail_buffer);
        return *this;
    }

    void upgrade(Planetoid&& a) {
        color = a.color;
        position = a.position;
        radius = a.radius;
        mass = a.mass;
        velocity = a.velocity;
    }

    void update_trail_buffer() {
        trail.insert(trail.begin(), position);
        if(trail.size() > trail_lim) trail.erase(trail.begin() + trail_lim, trail.end());

        trail_buffer.set_data(&trail[0], trail.size(), sizeof(float) * 2, GL_DYNAMIC_DRAW);
        trail_buffer.set_attrib(0, 2, sizeof(float) * 2, 0);
    }

    ~Planetoid() noexcept = default;
};

double core_x_dist = 0x1800;
double core_y_dist = 0x1800;

int stars_in_core = 0x100;

double arm_x_dist = 0x4000;
double arm_y_dist = 0x1000;
double arm_x_mean = 0x7000;
double arm_y_mean = 0x0;
double arm_x_offset = 0x0;
// x_mean should be x_dist * 0x1.8 + x_offset for a barred spiral galaxy

int stars_per_arm = 0x200;

double spiral = 1.5;
int arms = 3;

float gaussian_random(float mean, float stdev) {
    float u = float(rand()) / 32768;
    float v = float(rand()) / 32768;
    
    return (sqrt(-2 * std::log(u)) * cos(2.0 * M_PI * v)) * stdev + mean;
}

glm::vec2 spiral_func(glm::vec2 pos, double offset) {
    float r = glm::length(pos);
    double theta = offset;
    theta += (pos.x > 0) ? atan(pos.y / pos.x) : atan(pos.y / pos.x) + M_PI;
    theta += (std::max(r - arm_x_offset, 0.0) / (arm_x_dist)) * spiral;
    return {r * cos(theta), r * sin(theta)};
}

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

    std::unordered_map<GLuint, bool> mouse_button_map = {
        {GLFW_MOUSE_BUTTON_LEFT, false}
    };

    void init() {
        //const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        //monitor_size = {mode->width, mode->height};

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
    }

    void game_loop();
};

Core core;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    core.screen_size.x = width;
    core.screen_size.y = height;
    core.viewport_size.x = width + 1 * (width & 1);
    core.viewport_size.y = height + 1 * (height & 1);

    glViewport(0, 0, core.viewport_size.x, core.viewport_size.y);

    core.screen_matrix = glm::translate(glm::scale(identity_matrix, glm::vec2(2.0 / core.viewport_size.x, 2.0 / core.viewport_size.y)), glm::vec2(-core.viewport_size / 2));

    core.game_loop();
}

void char_callback(GLFWwindow* window, unsigned int codepoint) {

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    glm::ivec2 new_cursor_pos = {xpos, core.screen_size.y - ypos - 1};

    if(core.mouse_button_map[GLFW_MOUSE_BUTTON_LEFT]) {
        glm::dvec2 difference = new_cursor_pos - core.cursor_pos;

        core.camera_pos -= difference / (16 * core.scale);
    }

    core.cursor_pos = new_cursor_pos;
}

//void click();

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(core.mouse_button_map.contains(button)) {
        if(action == GLFW_PRESS) {
            core.mouse_button_map[button] = true;
            //click();
        } else if(action == GLFW_RELEASE) {
            core.mouse_button_map[button] = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if(yoffset > 0) {
        core.scale *= 1.2;
    } else if(yoffset < 0) {
        core.scale /= 1.2;
    }
}

struct Node {
    int id = -1;
    glm::vec2 center_of_mass = {0, 0};
    double total_mass = 0;
    std::array<std::unique_ptr<Node>, 4> children;
};

Node root;

void bh_recursive(Node* active_node, double radius, glm::dvec2 center, int i) {
    if(active_node->id == -1) {
        active_node->id = i;
        active_node->center_of_mass = core.planetoids[i].position;
        active_node->total_mass = core.planetoids[i].mass;
    } else if(active_node->id == -2) {
        active_node->center_of_mass = (glm::dvec2(active_node->center_of_mass) * active_node->total_mass + core.planetoids[i].position * core.planetoids[i].mass) / (active_node->total_mass + core.planetoids[i].mass);
        active_node->total_mass += core.planetoids[i].mass;

        bool ns = core.planetoids[i].position.y >= center.y;
        bool we = core.planetoids[i].position.x >= center.x;
        center = center - glm::dvec2{radius * 0.5, radius * 0.5} + glm::dvec2{radius * we, radius * ns};
        std::unique_ptr<Node>& new_ptr = active_node->children[we + ns * 2];
        if(!new_ptr) new_ptr = std::unique_ptr<Node>(new Node);
        radius /= 2;
        bh_recursive(new_ptr.get(), radius, center, i);
    } else {
        int other_id = active_node->id;
        active_node->id = -2;
        active_node->center_of_mass = (glm::dvec2(active_node->center_of_mass) * active_node->total_mass + core.planetoids[i].position * core.planetoids[i].mass) / (active_node->total_mass + core.planetoids[i].mass);
        active_node->total_mass += core.planetoids[i].mass;

        bool ns = core.planetoids[i].position.y >= center.y;
        bool we = core.planetoids[i].position.x >= center.x;
        glm::dvec2 new_center = center - glm::dvec2{radius * 0.5, radius * 0.5} + glm::dvec2{radius * we, radius * ns};
        std::unique_ptr<Node>& new_ptr = active_node->children[we + ns * 2];
        if(!new_ptr) new_ptr = std::unique_ptr<Node>(new Node);
        double new_radius = radius / 2;
        bh_recursive(new_ptr.get(), new_radius, new_center, i);

        ns = core.planetoids[other_id].position.y >= center.y;
        we = core.planetoids[other_id].position.x >= center.x;
        new_center = center - glm::dvec2{radius * 0.5, radius * 0.5} + glm::dvec2{radius * we, radius * ns};
        std::unique_ptr<Node>& other_new_ptr = active_node->children[we + ns * 2];
        if(!other_new_ptr) other_new_ptr = std::unique_ptr<Node>(new Node);
        bh_recursive(other_new_ptr.get(), new_radius, new_center, other_id);
    }
}

//std::vector<glm::vec2> vertices;

/*void recursive_search(Node* current_node, int depth) {
    if(current_node->id > -1) {
        if(depth != 0) {
            double pow_2 = pow(2, floor(log2(quadtree_rad)) + 1 - depth);
            glm::dvec2 corner = glm::floor(current_node->center_of_mass / float(pow_2)) * float(pow_2);

            std::array<glm::vec2, 8> array = {
                glm::vec2(corner),
                glm::vec2(corner) + glm::vec2(pow_2, 0),
                glm::vec2(corner) + glm::vec2(pow_2, 0),
                glm::vec2(corner) + glm::vec2(pow_2, pow_2),
                glm::vec2(corner) + glm::vec2(pow_2, pow_2),
                glm::vec2(corner) + glm::vec2(0, pow_2),
                glm::vec2(corner) + glm::vec2(0, pow_2),
                glm::vec2(corner)
            };

            vertices.insert(vertices.end(), array.begin(), array.end());
        }
    } else {
        if(current_node->children[0]) recursive_search(current_node->children[0].get(), depth + 1);
        if(current_node->children[1]) recursive_search(current_node->children[1].get(), depth + 1);
        if(current_node->children[2]) recursive_search(current_node->children[2].get(), depth + 1);
        if(current_node->children[3]) recursive_search(current_node->children[3].get(), depth + 1);
    }
}*/

void recursive_gravity(Node* current_node, int id, double width) {
    if(current_node->id < 0) {
        glm::dvec2 diff = current_node->center_of_mass - glm::vec2(core.planetoids[id].position);
        double dist = glm::length(diff);
        if(width / dist < 0.5) {
            core.planetoids[id].accel += G * current_node->total_mass / (dist * dist * dist) * diff;
        } else {
            double new_width = width / 2;
            if(current_node->children[0]) recursive_gravity(current_node->children[0].get(), id, new_width);
            if(current_node->children[1]) recursive_gravity(current_node->children[1].get(), id, new_width);
            if(current_node->children[2]) recursive_gravity(current_node->children[2].get(), id, new_width);
            if(current_node->children[3]) recursive_gravity(current_node->children[3].get(), id, new_width);
        }
    } else {
        if(current_node->id != id) {
            glm::dvec2 diff = current_node->center_of_mass - glm::vec2(core.planetoids[id].position);
            double dist = glm::length(diff);
            core.planetoids[id].accel += G * current_node->total_mass / (dist * dist * dist) * diff;
        }
    }
}

double trail_buffer_counter = 0;

void calculate(double delta_time) {
    for(int id = 0; id < core.planetoids.size(); ++id) {
        recursive_gravity(&root, id, quadtree_rad * 2);
        
        Planetoid& p = core.planetoids[id];
        p.velocity += p.accel * simulation_speed * delta_time;
        p.position += p.velocity * simulation_speed * delta_time;
        p.accel = {0, 0};
    }

    bool update_trails = false;
    trail_buffer_counter += delta_time * simulation_speed;
    if(trail_buffer_counter > 50.0) {
        trail_buffer_counter = std::fmod(trail_buffer_counter, 50.0);
        update_trails = true;
    }

    for(int i = 0; i < core.planetoids.size() - 1; ++i) {
        Planetoid& p0 = core.planetoids[i];
        /*if(p0.position.x > quadtree_rad) {
            p0.position.x = -quadtree_rad;
        } else if(p0.position.x < -quadtree_rad) {
            p0.position.s = quadtree_rad;
        }
        if(p0.position.y > quadtree_rad) {
            p0.position.y = -quadtree_rad;
        } else if(p0.position.y < -quadtree_rad) {
            p0.position.y = quadtree_rad;
        }*/
        if(abs(p0.position.x) >= quadtree_rad || abs(p0.position.y) >= quadtree_rad) {
            core.planetoids.erase(core.planetoids.begin() + i);
            i--;
            continue;
        }
        for(int j = i + 1; j < core.planetoids.size(); ++j) {
            Planetoid& p1 = core.planetoids[j];

            glm::dvec2 dist = p1.position - p0.position;
            double collide_limit = p0.radius + p1.radius;

            if(dist.x * dist.x + dist.y * dist.y < collide_limit * collide_limit) {
                Planetoid& massive_planetoid = (p1.mass > p0.mass) ? p1 : p0;
                Planetoid& larger_planetoid = (p1.radius > p0.radius) ? p1 : p0;
                larger_planetoid.upgrade(Planetoid{massive_planetoid.color, larger_planetoid.position, cbrt(p0.mass + p1.mass), p0.mass + p1.mass, glm::dvec2(p0.velocity * p0.mass + p1.velocity * p1.mass) / (p0.mass + p1.mass)});
                if(&p0 != &larger_planetoid) p0 = std::move(larger_planetoid);
                //if(p0.mass >= 0x200) p0.color = {1.0, std::min(1.0, double(p0.mass - 0x200) / 0xE00), 0.0, 1.0};
                core.planetoids.erase(core.planetoids.begin() + j);
                j--;

                break;
            }
        }
        if(update_trails) p0.update_trail_buffer();
        else {
            p0.trail_buffer.bind();
            p0.trail[0] = p0.position;

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 2, &p0.trail[0]);
        }
    }
    if(core.planetoids.size() != 0) {
        Planetoid& p0 = core.planetoids.back();
        if(update_trails) p0.update_trail_buffer();
        else {
            p0.trail_buffer.bind();
            p0.trail[0] = p0.position;

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 2, &p0.trail[0]);
        }
    }
}

void barnes_hut() {
    root = Node();
    for(int i = 0; i < core.planetoids.size(); i++) {
        double radius = quadtree_rad;
        glm::dvec2 center = {0, 0};

        bh_recursive(&root, radius, center, i);
    }

    /*recursive_search(&root, 0);
    
    core.quadtree_buffer.set_data(vertices.data(), vertices.size(), sizeof(glm::vec2));
    core.quadtree_buffer.set_attrib(0, 2, sizeof(float) * 2, 0);

    vertices.clear();*/
}

/*void click_recursive(glm::vec2& cursor_pos, Node* current_node, glm::vec2 center, int r) {
    bool ns = cursor_pos.y >= center.y;
    bool we = cursor_pos.x >= center.x;
    if() {

    }
}

void click() {
    glm::mat3 matrix = glm::translate(glm::scale(glm::translate(identity_matrix, glm::vec2(core.camera_pos)), {1.0 / (16 * core.scale), 1.0 / (16 * core.scale)}), glm::vec2(-core.screen_size / 2));
    glm::vec2 cursor_world_pos = (matrix * glm::vec3(core.cursor_pos, 1.0)).xy();
    //std::cout << cursor_world_pos.x << " " << cursor_world_pos.y << "\n";

    if(abs(cursor_world_pos.x) < quadtree_rad && abs(cursor_world_pos.x) < quadtree_rad) {
        click_recursive(cursor_world_pos, &root, {0, 0}, quadtree_rad);
    }
}*/

void Core::game_loop() {
    uint64_t current_time = get_time();
    double delta_time = double(current_time - time) / 1000;
    time = current_time;

    if(!sim_paused) {
        barnes_hut();
        calculate(delta_time);
    }
    //std::cout << delta_time << "\n";

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::vec4 color = {0.5, 0.0, 1.0, 1.0};

    glm::mat3 view_matrix = glm::translate(glm::scale(identity_matrix, {16 * scale, 16 * scale}), glm::vec2{1, 1}) * screen_matrix * glm::translate(identity_matrix, glm::vec2{-camera_pos});

    for(Planetoid& p : planetoids) {
        p.trail_buffer.bind();
        flat_shader.use();
        glUniformMatrix3fv(0, 1, false, &identity_matrix[0][0]);
        glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
        glUniform4fv(2, 1, &p.color[0]);
        glDrawArrays(GL_LINE_STRIP, 0, p.trail_buffer.vertices);


        if(p.radius * scale * 16 > 1) {
            circle_buffer.bind();
            shader_circle.use();
            glm::mat3 transformation_matrix = glm::scale(glm::translate(identity_matrix, glm::vec2{p.position}), {p.radius, p.radius});
            glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &transformation_matrix[0][0]);
            glUniform4fv(2, 1, &p.color[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        } else {
            glm::vec2 screen_pos = (glm::translate(glm::scale(glm::translate(identity_matrix, glm::vec2(core.viewport_size / 2)), {16 * scale, 16 * scale}), glm::vec2(-core.camera_pos)) * glm::vec3(p.position, 1.0)).xy();
            glm::mat3 matrix = glm::translate(identity_matrix, glm::floor(screen_pos));
            x_buffer.bind();
            flat_shader.use();
            glUniformMatrix3fv(0, 1, false, &matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &screen_matrix[0][0]);
            glUniform4fv(2, 1, &p.color[0]);
            glDrawArrays(GL_LINES, 0, x_buffer.vertices);
        }
    }

    core.quadtree_buffer.bind();
    core.flat_shader.use();
    glUniformMatrix3fv(0, 1, false, &identity_matrix[0][0]);
    glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
    glUniform4f(2, 1.0, 0.0, 0.0, 1.0);
    glDrawArrays(GL_LINES, 0, core.quadtree_buffer.vertices);
    
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
    glfwSetScrollCallback(core.window, scroll_callback);
    glfwSetMouseButtonCallback(core.window, mouse_button_callback);

    core.init();

    std::array<glm::vec2, 8> array = {
        glm::vec2(-quadtree_rad, -quadtree_rad),
        glm::vec2(quadtree_rad, -quadtree_rad),
        glm::vec2(quadtree_rad, -quadtree_rad),
        glm::vec2(quadtree_rad, quadtree_rad),
        glm::vec2(quadtree_rad, quadtree_rad),
        glm::vec2(-quadtree_rad, quadtree_rad),
        glm::vec2(-quadtree_rad, quadtree_rad),
        glm::vec2(-quadtree_rad, -quadtree_rad)
    };

    core.quadtree_buffer.set_data(array.data(), array.size(), sizeof(glm::vec2));
    core.quadtree_buffer.set_attrib(0, 2, sizeof(float) * 2, 0);

    core.time = get_time();
    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}