#include "core.cpp"

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    core.framebuffer_update(width, height);
}

void char_callback(GLFWwindow* window, unsigned int codepoint) {
    core.char_update(codepoint);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    core.key_update(key, action);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    core.cursor_pos_update(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    core.mouse_button_update(button, action);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    core.scroll_update(yoffset);
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

void barnes_hut() {
    root = Node();
    for(int i = 0; i < core.planetoids.size(); i++) {
        double radius = quadtree_rad;
        glm::dvec2 center = {0, 0};

        bh_recursive(&root, radius, center, i);
    }
}

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

void calculate(double delta_time) {
    if(!sim_paused) {
        barnes_hut();

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

        for(int i = 0; i < core.planetoids.size(); ++i) {
            Planetoid& p0 = core.planetoids[i];

            if(abs(p0.position.x) >= quadtree_rad || abs(p0.position.y) >= quadtree_rad) {
                if(core.selected_planetoid == i) core.selected_planetoid = -1;
                else if(core.selected_planetoid > i) --core.selected_planetoid;

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

                    double new_mass = p0.mass + p1.mass;
                    larger_planetoid.upgrade(Planetoid{p0.color * float(p0.mass / new_mass) + p1.color * float(p1.mass / new_mass), larger_planetoid.position, cbrt(new_mass), new_mass, glm::dvec2(p0.velocity * p0.mass + p1.velocity * p1.mass) / new_mass});
                    
                    
                    if(&p0 != &larger_planetoid) {
                        p0 = std::move(larger_planetoid);
                        if(core.selected_planetoid == j) core.selected_planetoid = i;
                        else if(core.selected_planetoid > j) --core.selected_planetoid;
                    } else {
                        if(core.selected_planetoid == j) core.selected_planetoid = -1;
                        else if(core.selected_planetoid > j) --core.selected_planetoid;
                    }
                    
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
    }

    if(core.key_map[GLFW_KEY_W]) {
        core.view_pos += core.view_dir * float(core.move_speed * delta_time);
    }
    if(core.key_map[GLFW_KEY_S]) {
        core.view_pos -= core.view_dir * float(core.move_speed * delta_time);
    }
    glm::vec3 sideways = glm::normalize(glm::cross(core.view_dir, core.up_dir));
    if(core.key_map[GLFW_KEY_D]) {
        core.view_pos += sideways * float(core.move_speed * delta_time);
    }
    if(core.key_map[GLFW_KEY_A]) {
        core.view_pos -= sideways * float(core.move_speed * delta_time);
    }
    if(core.key_map[GLFW_KEY_E]) {
        core.up_dir = glm::rotate(identity_matrix_4, float(2 * M_PI * (delta_time / 4000)), core.view_dir) * glm::vec4(core.up_dir, 1.0);
    }
    if(core.key_map[GLFW_KEY_Q]) {
        core.up_dir = glm::rotate(identity_matrix_4, float(2 * M_PI * -(delta_time / 4000)), core.view_dir) * glm::vec4(core.up_dir, 1.0);
    }
    if(core.key_map[GLFW_KEY_MINUS]) {
        core.theta -= 2 * M_PI * 0.002 * float(core.move_speed * delta_time);
    }
    if(core.key_map[GLFW_KEY_EQUAL]) {
        core.theta += 2 * M_PI * 0.002 * float(core.move_speed * delta_time);
    }

    //std::cout << core.view_pos.x << " " << core.view_pos.y << "\n";
}

void Core::game_loop() {
    uint64_t current_time = get_time();
    double delta_time = double(current_time - time) / 1000;
    time = current_time;

    calculate(delta_time);

    if(stars.size() != num_planetoids) {
        num_planetoids = stars.size();

        std::get<1>(gui_core.widgets[0]).load_buffers(gui_core.font, "Stars: " + to_base(num_planetoids, 16));
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec4 color = {0.5, 0.0, 1.0, 1.0};

    glm::mat3 view_matrix = glm::translate(glm::scale(identity_matrix_3, {16 * scale, 16 * scale}), glm::vec2{1, 1}) * screen_matrix * glm::translate(identity_matrix_3, glm::vec2{-camera_pos});

    for(int i = 0; i < planetoids.size(); i++) {
        Planetoid& p = planetoids[i];

        glm::vec4 color;
        if(i == core.selected_planetoid) {
            color = {1.0, 0.0, 0.0, 1.0};
        } else {
            color = p.color;
        }

        p.trail_buffer.bind();
        flat_shader.use();
        glUniformMatrix3fv(0, 1, false, &identity_matrix_3[0][0]);
        glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
        glUniform4fv(2, 1, &color[0]);
        glDrawArrays(GL_LINE_STRIP, 0, p.trail_buffer.vertices);

        if(p.radius * scale * 16 > 1) {
            circle_buffer.bind();
            shader_circle.use();
            glm::mat3 transformation_matrix = glm::scale(glm::translate(identity_matrix_3, glm::vec2{p.position}), {p.radius, p.radius});
            glUniformMatrix3fv(0, 1, false, &view_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &transformation_matrix[0][0]);
            glUniform4fv(2, 1, &color[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        } else {
            glm::vec2 screen_pos = (glm::translate(glm::scale(glm::translate(identity_matrix_3, glm::vec2(core.viewport_size / 2)), {16 * scale, 16 * scale}), glm::vec2(-core.camera_pos)) * glm::vec3(p.position, 1.0)).xy();
            glm::mat3 matrix = glm::translate(identity_matrix_3, glm::floor(screen_pos));
            x_buffer.bind();
            flat_shader.use();
            glUniformMatrix3fv(0, 1, false, &matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &screen_matrix[0][0]);
            glUniform4fv(2, 1, &color[0]);
            glDrawArrays(GL_LINES, 0, x_buffer.vertices);
        }
    }

    glm::mat4 view = glm::lookAt(view_pos, view_pos + view_dir, up_dir);
    glm::mat4 projection = glm::perspective(float(2 * M_PI * 0x0.4p0), float(screen_size.x) / screen_size.y, 0x0.1p0f, 0x400000.0p0f);

    /*grid_shader.use();
    glUniformMatrix4fv(0, 1, false, &view[0][0]);
    glUniformMatrix4fv(1, 1, false, &projection[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);*/

    for(Star& s : stars) {
        point_shader.use();
        glUniform3fv(0, 1, &s.position[0]);
        glUniformMatrix4fv(1, 1, false, &view[0][0]);
        glUniformMatrix4fv(2, 1, false, &projection[0][0]);
        glUniform4fv(3, 1, &s.color[0]);
        glDrawArrays(GL_POINTS, 0, 1);
        
        /*glm::vec4 new_pos_4 = projection * (view * glm::vec4(s.position, 1.0));
        if(abs(new_pos_4.x) <= new_pos_4.w  && abs(new_pos_4.y) <= new_pos_4.w && abs(new_pos_4.z) <= new_pos_4.w) {
            glm::vec3 new_pos = glm::vec3{new_pos_4.x, new_pos_4.y, -new_pos_4.z} / new_pos_4.w;
            circle_buffer.bind();
            shader_circle.use();
            glm::mat3 transformation_matrix = glm::translate(identity_matrix_3, glm::floor(new_pos.xy() * glm::vec2(core.screen_size / 2) + glm::vec2(core.screen_size) / 2.0f));
            glUniformMatrix3fv(0, 1, false, &screen_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &transformation_matrix[0][0]);
            glUniform4fv(2, 1, &s.color[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }*/
    }

    /*line_shader.use();
    glUniformMatrix4fv(2, 1, false, &view[0][0]);
    glUniformMatrix4fv(3, 1, false, &projection[0][0]);

    glUniform4f(4, 1.0, 0.0, 0.0, 1.0);
    glUniform3f(0, 0, 0, 0);
    glUniform3f(1, 0x100 * cos(theta), 0x100 * sin(theta), 0);
    glDrawArrays(GL_LINES, 0, 2);

    float dist = (theta * arm_x_dist) / spiral;
    glm::vec3 pt = {dist * cos(theta), dist * sin(theta), 0};
    float theta_1 = theta + M_PI / 2;
    float theta_2 = theta + atan(theta) + M_PI / 2;

    glUniform4f(4, 0.0, 1.0, 0.0, 1.0);
    glUniform3f(0, pt.x + 0x100 * cos(theta_1), pt.y + 0x100 * sin(theta_1), 0);
    glUniform3f(1, pt.x - 0x100 * cos(theta_1), pt.y - 0x100 * sin(theta_1), 0);
    glDrawArrays(GL_LINES, 0, 2);

    glUniform4f(4, 1.0, 0.0, 1.0, 1.0);
    glUniform3f(0, pt.x + 0x100 * cos(theta_2), pt.y + 0x100 * sin(theta_2), 0);
    glUniform3f(1, pt.x - 0x100 * cos(theta_2), pt.y - 0x100 * sin(theta_2), 0);
    glDrawArrays(GL_LINES, 0, 2);*/

    core.quadtree_buffer.bind();
    core.flat_shader.use();
    glUniformMatrix3fv(0, 1, false, &identity_matrix_3[0][0]);
    glUniformMatrix3fv(1, 1, false, &view_matrix[0][0]);
    glUniform4f(2, 1.0, 0.0, 0.0, 1.0);
    glDrawArrays(GL_LINES, 0, core.quadtree_buffer.vertices);

    gui_core.draw();
    
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

    glEnable(GL_DEPTH_TEST);

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