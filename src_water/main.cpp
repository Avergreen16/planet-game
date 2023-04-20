#include "core.cpp"

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

struct time_counter {
    std::time_t t = get_time();
    std::time_t d = 0;

    void tick() {
        std::time_t a = get_time();
        d = a - t;
        t = a;
    }
};
time_counter t;

void calculate(double delta_time) {
    for(event& e : core.gui_core.events) {
        switch(e.index()) {
            case 0: {
                key_event& k = std::get<0>(e);

                if(k.key == GLFW_KEY_ESCAPE) {
                    if(k.action == GLFW_PRESS) {
                        if(core.cursor_hidden) {
                            core.cursor_hidden = false;
                            glfwSetInputMode(core.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        } else {
                            core.cursor_hidden = true;
                            glfwSetInputMode(core.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                        }
                    }
                } else if(k.key == GLFW_KEY_MINUS) {
                    if(k.action == GLFW_PRESS) {
                        std::vector<glm::ivec3> keys;
                        for(auto& [k, c] : core.chunks) {
                            keys.push_back(k);
                        }

                        core.gen_queue_mutex.lock();
                        core.chunk_allocate_main_mutex.lock();
                        core.buffer_update_mutex.lock();

                        for(glm::ivec3 k : keys) {
                            core.chunks.erase(k);
                        }
                        core.chunk_buffer_updates.clear();
                        core.chunk_gen_queue = decltype(core.chunk_gen_queue)();

                        core.gen_queue_mutex.unlock();
                        core.chunk_allocate_main_mutex.unlock();
                        core.buffer_update_mutex.unlock();
                    }
                } else if(k.key == GLFW_KEY_EQUAL) {
                    if(k.action == GLFW_PRESS) {
                        core.space_core.sim_active = !core.space_core.sim_active;
                    }
                }
                break;
            }
            case 1: {
                mouse_button_event& m = std::get<1>(e);

                if(m.button == GLFW_MOUSE_BUTTON_LEFT) {
                    if(m.action == GLFW_PRESS) {
                        glm::ivec3 pos;
                        if(raycast(core.view_pos, core.view_dir, pos, 5.0f)) {
                            get_block(pos) = 0;

                            glm::ivec3 i = glm::floor(glm::vec3(pos) / 32.0f);
                            glm::ivec3 chunk_i = mod(pos, 32);
                            core.chunks[i].status = 3;
                            core.chunk_gen_queue.push(i);

                            if(chunk_i.x == 0) {
                                glm::ivec3 key = i + glm::ivec3{-1, 0, 0};
                                core.chunks[key].status = 3;
                                core.chunk_gen_queue.push(key);
                            } else if(chunk_i.x == 31) {
                                glm::ivec3 key = i + glm::ivec3{1, 0, 0};
                                core.chunks[key].status = 3;
                                core.chunk_gen_queue.push(key);
                            }
                            if(chunk_i.y == 0) {
                                glm::ivec3 key = i + glm::ivec3{0, -1, 0};
                                core.chunks[key].status = 3;
                                core.chunk_gen_queue.push(key);
                            } else if(chunk_i.y == 31) {
                                glm::ivec3 key = i + glm::ivec3{0, 1, 0};
                                core.chunks[key].status = 3;
                                core.chunk_gen_queue.push(key);
                            }
                            if(chunk_i.z == 0) {
                                glm::ivec3 key = i + glm::ivec3{0, 0, -1};
                                core.chunks[key].status = 3;
                                core.chunk_gen_queue.push(key);
                            } else if(chunk_i.z == 31) {
                                glm::ivec3 key = i + glm::ivec3{0, 0, 1};
                                core.chunks[key].status = 3;
                                core.chunk_gen_queue.push(key);
                            }
                        }
                    }
                } else if(m.button == GLFW_MOUSE_BUTTON_RIGHT) {
                    if(m.action == GLFW_PRESS) {
                        glm::ivec3 pos;
                        if(raycast_place(core.view_pos, core.view_dir, pos, 5.0f)) {
                            uint16_t& block = get_block(pos);
                            if(block == 0) {
                                block = 1;
                                core.block_updates.insert(pos);

                                glm::ivec3 i = glm::floor(glm::vec3(pos) / 32.0f);
                                glm::ivec3 chunk_i = mod(pos, 32);
                                core.chunks[i].status = 3;
                                core.chunk_gen_queue.push(i);

                                if(chunk_i.x == 0) {
                                    glm::ivec3 key = i + glm::ivec3{-1, 0, 0};
                                    core.chunks[key].status = 3;
                                    core.chunk_gen_queue.push(key);
                                } else if(chunk_i.x == 31) {
                                    glm::ivec3 key = i + glm::ivec3{1, 0, 0};
                                    core.chunks[key].status = 3;
                                    core.chunk_gen_queue.push(key);
                                }
                                if(chunk_i.y == 0) {
                                    glm::ivec3 key = i + glm::ivec3{0, -1, 0};
                                    core.chunks[key].status = 3;
                                    core.chunk_gen_queue.push(key);
                                } else if(chunk_i.y == 31) {
                                    glm::ivec3 key = i + glm::ivec3{0, 1, 0};
                                    core.chunks[key].status = 3;
                                    core.chunk_gen_queue.push(key);
                                }
                                if(chunk_i.z == 0) {
                                    glm::ivec3 key = i + glm::ivec3{0, 0, -1};
                                    core.chunks[key].status = 3;
                                    core.chunk_gen_queue.push(key);
                                } else if(chunk_i.z == 31) {
                                    glm::ivec3 key = i + glm::ivec3{0, 0, 1};
                                    core.chunks[key].status = 3;
                                    core.chunk_gen_queue.push(key);
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    if(core.key_map[GLFW_KEY_E]) {
        core.up_dir = glm::rotate(identity_matrix_4, float(2 * M_PI * (delta_time / 3)), core.view_dir) * glm::vec4(core.up_dir, 1.0);
    }
    if(core.key_map[GLFW_KEY_Q]) {
        core.up_dir = glm::rotate(identity_matrix_4, float(2 * M_PI * -(delta_time / 3)), core.view_dir) * glm::vec4(core.up_dir, 1.0);
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
    if(core.key_map[GLFW_KEY_SPACE]) {
        core.view_pos += core.up_dir * float(core.move_speed * delta_time);  
    }
    if(core.key_map[GLFW_KEY_LEFT_SHIFT]) {
        core.view_pos -= core.up_dir * float(core.move_speed * delta_time);
    }
    
    core.chunk_allocate_thread_mutex.lock();
    if(raycast(core.view_pos, core.view_dir, core.selected_block, 5.0f)) {
        core.block_selected = true;
    } else {
        core.block_selected = false;
    }
    core.chunk_allocate_thread_mutex.unlock();

    glm::ivec3 new_coord = glm::round(core.view_pos);
    if(core.current_coordinate != new_coord) {
        core.current_coordinate = new_coord;
        std::get<1>(core.gui_core.widgets[1]).load_buffers(core.gui_core.font, "Coordinates: " + std::to_string((int)core.current_coordinate.x) + " " + std::to_string((int)core.current_coordinate.y) + " " + std::to_string((int)core.current_coordinate.z));
    }

    glm::ivec3 new_index = glm::floor(core.view_pos / 32.0f);
    if(core.current_index != new_index) {
        core.gen_queue_mutex.lock();
        core.current_index = new_index;
        std::get<1>(core.gui_core.widgets[2]).load_buffers(core.gui_core.font, "Chunk: " + std::to_string(core.current_index.x) + " " + std::to_string(core.current_index.y) + " " + std::to_string(core.current_index.z));
        for(int x = -region.x; x <= region.x; ++x) {
            for(int y = -region.y; y <= region.y; ++y) {
                for(int z = -region.z; z <= region.z; ++z) {
                    glm::ivec3 key = new_index + glm::ivec3{x, y, z};
                    core.chunk_gen_queue.push(key);
                }
            }
        }
        core.gen_queue_mutex.unlock();
    }

    core.chunk_allocate_thread_mutex.lock();
    for(glm::ivec3 key : core.chunk_buffer_updates) {
        core.chunks[key].load_buffers();
    }
    core.chunk_allocate_thread_mutex.unlock();

    
    core.buffer_update_mutex.lock();
    core.chunk_buffer_updates.clear();
    core.buffer_update_mutex.unlock();

    uint8_t new_dir;
    glm::vec3 norm_view_dir = glm::abs(core.view_dir);
    if(norm_view_dir.x < norm_view_dir.y) {
        if(norm_view_dir.y < norm_view_dir.z) {
            if(core.view_dir.z > 0) {
                new_dir = UP;
            } else {
                new_dir = DOWN;
            }
        } else {
            if(core.view_dir.y > 0) {
                new_dir = NORTH;
            } else {
                new_dir =  SOUTH;
            }
        }
    } else {
        if(norm_view_dir.x < norm_view_dir.z) {
            if(core.view_dir.z > 0) {
                new_dir = UP;
            } else {
                new_dir = DOWN;
            }
        } else {
            if(core.view_dir.x > 0) {
                new_dir = EAST;
            } else {
                new_dir = WEST;
            }
        }
    }

    if(new_dir != core.dir_enum) {
        core.dir_enum = new_dir;
        switch(core.dir_enum) {
            case NORTH: {
                std::get<1>(core.gui_core.widgets[4]).load_buffers(core.gui_core.font, "Direction: +y (north)");
                break;
            }
            case EAST: {
                std::get<1>(core.gui_core.widgets[4]).load_buffers(core.gui_core.font, "Direction: +x (east)");
                break;
            }
            case SOUTH: {
                std::get<1>(core.gui_core.widgets[4]).load_buffers(core.gui_core.font, "Direction: -y (south)");
                break;
            }
            case WEST: {
                std::get<1>(core.gui_core.widgets[4]).load_buffers(core.gui_core.font, "Direction: -x (west)");
                break;
            }
            case UP: {
                std::get<1>(core.gui_core.widgets[4]).load_buffers(core.gui_core.font, "Direction: +z (up)");
                break;
            }
            case DOWN: {
                std::get<1>(core.gui_core.widgets[4]).load_buffers(core.gui_core.font, "Direction: -z (down)");
                break;
            }
        }
    }

    core.tick_time += delta_time;

    /*if(core.tick_time > 0x0.4p0) {
        core.tick_time = 0.0;

        auto set = std::move(core.block_updates);
        core.block_updates.clear();

        for(glm::ivec3 i : set) {
            block_update(i);
        }

        for(glm::ivec3 i : core.changed_chunks) {
            core.chunks[i].load_buffers();
        }
        core.changed_chunks.clear();
    }*/

    core.frame_time += delta_time;
    ++core.frame_count;

    if(core.frame_time >= 1.0) {
        std::get<1>(core.gui_core.widgets[0]).load_buffers(core.gui_core.font, "FPS: " + std::to_string((int)(core.frame_count * (1.0 / core.frame_time))));
        core.frame_count = 0;
        core.frame_time = 0.0;
    }

    core.gui_core.events.clear();
}

void Space_core::draw(glm::mat4 view_mat, glm::mat4 proj_mat) {
    for(Planet& p : planets) {
        float diff = (double(t - start_time) / 1000000);
        glm::mat4 trans_mat = glm::rotate(float((M_PI * 2) * diff / p.rotation), glm::vec3(0, 0, 1));
        trans_mat = glm::translate(p.position) * trans_mat;

        p.buffer.bind();

        if(p.use_color) {
            planet_shader_solid.use();
            glUniform3fv(4, 1, &p.color[0]);
        } else {
            planet_shader.use();
            textures[p.surface_index].bind(0);
        }

        glUniformMatrix4fv(0, 1, false, &trans_mat[0][0]);
        glUniformMatrix4fv(1, 1, false, &view_mat[0][0]);
        glUniformMatrix4fv(2, 1, false, &proj_mat[0][0]);

        if(p.is_star) {
            glUniform1i(5, 1);
        } else {
            glUniform1i(5, 0);

            glm::vec3 rel_light_pos = normalize(light_pos - p.position);
            glUniform3fv(3, 1, &rel_light_pos[0]);
        }
        
        glDrawArrays(GL_TRIANGLES, 0, p.buffer.vertices);
    }
}

void Core::game_loop() {

    glEnable(GL_DEPTH_TEST);

    uint64_t current_time = get_time();
    double delta_time = double(current_time - time) / 1000000 * (86400 / 0x10000);
    time = current_time;

    calculate(delta_time);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gui_framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    



    glm::mat4 view = glm::lookAt(view_pos, view_pos + view_dir, up_dir);
    glm::mat4 projection = glm::perspective(float(2 * M_PI * 0x0.4p0), float(screen_size.x) / screen_size.y, 0x0.1p0f, 0x800.0p0f);

    glm::mat4 rot_mat = glm::rotate(float((M_PI * 2) * (double(space_core.t - space_core.start_time) / 1000000) / space_core.planets[3].rotation), glm::vec3(0, 0, 1));
    
    glm::vec3 surface_vec = rot_mat * glm::vec4(glm::normalize(glm::vec3(-0.5, -0.3, -0.1)) * (space_core.planets[3].radius + 0.15f), 1.0);

    glm::vec3 look_dir = glm::vec4(glm::normalize(cross(surface_vec, glm::vec3(0.0, 0.0, 1.0))), 1.0);
    glm::mat4 space_view = glm::lookAt(space_core.planets[3].position + surface_vec, space_core.planets[3].position + surface_vec + look_dir, surface_vec);

    glm::vec3 rel_sun_pos = space_view * glm::vec4(space_core.planets[0].position, 1.0);

    glm::vec3 sky_col = {0.4, 0.7, 1.0};
    sky_col *= std::max(0.0f, glm::dot(normalize(rel_sun_pos), glm::vec3(0, 1, 0)));
    space_core.framebuffer.bind();
    glClearColor(sky_col.x, sky_col.y, sky_col.z, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    space_view = (glm::lookAt(glm::vec3(0, 0, 0), view_dir, up_dir) * glm::inverse(glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1)))) * space_view;

    space_core.framebuffer.bind();
    space_core.draw(space_view, projection);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    space_core.framebuffer.color_tex[0].bind(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    screen_shader.use();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    chunk_shader.use();
    tex.bind(0);

    chunk_allocate_thread_mutex.lock();
    for(int x = current_index.x - region.x; x <= current_index.x + region.x; ++x) {
        for(int y = current_index.y - region.y; y <= current_index.y + region.y; ++y) {
            for(int z = current_index.z - region.z; z <= current_index.z + region.z; ++z) {
                glm::ivec3 key = {x, y, z};
                if(chunks.contains(key)) {
                    Chunk& c = chunks[key];
                    if(c.status == 2 || c.status == 3) {
                        c.buffer.bind();
                        glUniformMatrix4fv(0, 1, false, &view[0][0]);
                        glUniformMatrix4fv(1, 1, false, &projection[0][0]);
                        glUniform3f(2, key.x, key.y, key.z);
                        glUniform3fv(3, 1, &rel_sun_pos[0]);
                        glDrawArrays(GL_TRIANGLES, 0, c.buffer.vertices);
                    }
                }
            }
        }
    }
    chunk_allocate_thread_mutex.unlock();

    if(block_selected) {
        cube_shader.use();
        glUniformMatrix4fv(0, 1, false, &view[0][0]);
        glUniformMatrix4fv(1, 1, false, &projection[0][0]);
        glUniform3iv(2, 1, &selected_block[0]);
        glUniform3f(3, 0.0f, 0.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 24);
    }

    /*grid_shader.use();
    glUniformMatrix4fv(0, 1, false, &view[0][0]);
    glUniformMatrix4fv(1, 1, false, &projection[0][0]);
    glUniform1f(2, 0x0.1p0f);
    glUniform1f(3, 0x100.0p0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);*/

    /*grid_shader_xz.use();
    glUniformMatrix4fv(0, 1, false, &view[0][0]);
    glUniformMatrix4fv(1, 1, false, &projection[0][0]);
    glUniform1f(2, 0x0.1p0f);
    glUniform1f(3, 0x100.0p0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    grid_shader_yz.use();
    glUniformMatrix4fv(0, 1, false, &view[0][0]);
    glUniformMatrix4fv(1, 1, false, &projection[0][0]);
    glUniform1f(2, 0x0.1p0f);
    glUniform1f(3, 0x100.0p0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);*/

    gui_framebuffer.bind();

    gui_core.draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);

    screen_shader.use();
    gui_framebuffer.color_tex[0].bind(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(window);
}

void calc_normals() {
    for(int i = 0; i < 256; ++i) {
        glm::vec3 a, b, c, n;
        for(int j = 0; j < triangle_table[i].size(); j += 3) {
            a = midpoint_vertices[triangle_table[i][j]];
            b = midpoint_vertices[triangle_table[i][j + 1]];
            c = midpoint_vertices[triangle_table[i][j + 2]];

            n = glm::normalize(glm::cross(a - c, b - c));

            normal_table[i].push_back(n);
        }
    }
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
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_BACK, GL_LINE);

    core.init();

    calc_normals();

    core.time = get_time();
    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}