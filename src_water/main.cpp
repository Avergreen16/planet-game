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

void calculate(double delta_time) {
    for(event& e : core.gui_core.events) {
        switch(e.index()) {
            case 0: {
                key_event& k = std::get<0>(e);

                if(k.key == GLFW_KEY_EQUAL) {
                    if(k.action == GLFW_PRESS) {
                        glm::ivec3 i = floor(core.view_pos / 16.0f);
                        std::cout << "i = " << i.x << " " << i.y << " " << i.z << "\n";
                        glm::ivec3 chunk_i = mod(glm::floor(core.view_pos), 16);
                        (*core.chunks[i].data)[chunk_i.x][chunk_i.y][chunk_i.z] = 3;
                        core.chunks[i].load_buffers();

                        if(chunk_i.x == 0) {
                            core.chunks[i + glm::ivec3{-1, 0, 0}].load_buffers();
                        } else if(chunk_i.x == 15) {
                            core.chunks[i + glm::ivec3{1, 0, 0}].load_buffers();
                        }
                        if(chunk_i.y == 0) {
                            core.chunks[i + glm::ivec3{0, -1, 0}].load_buffers();
                        } else if(chunk_i.y == 15) {
                            core.chunks[i + glm::ivec3{0, 1, 0}].load_buffers();
                        }
                        if(chunk_i.z == 0) {
                            core.chunks[i + glm::ivec3{0, 0, -1}].load_buffers();
                        } else if(chunk_i.z == 15) {
                            core.chunks[i + glm::ivec3{0, 0, 1}].load_buffers();
                        }

                        core.block_updates.insert(i * 16 + chunk_i);
                    }
                }
                break;
            }
            case 1: {
                mouse_button_event& m = std::get<1>(e);

                if(m.button == GLFW_MOUSE_BUTTON_LEFT) {
                    if(m.action == GLFW_PRESS) {
                        glm::ivec3 pos;
                        if(raycast(core.view_pos, core.view_dir, pos, 6.0f)) {
                            get_block(pos) = 0;
                        }

                        glm::ivec3 i = glm::floor(glm::vec3(pos) / 16.0f);
                        glm::ivec3 chunk_i = mod(pos, 16);
                        core.chunks[i].load_buffers();

                        if(chunk_i.x == 0) {
                            core.chunks[i + glm::ivec3{-1, 0, 0}].load_buffers();
                        } else if(chunk_i.x == 15) {
                            core.chunks[i + glm::ivec3{1, 0, 0}].load_buffers();
                        }
                        if(chunk_i.y == 0) {
                            core.chunks[i + glm::ivec3{0, -1, 0}].load_buffers();
                        } else if(chunk_i.y == 15) {
                            core.chunks[i + glm::ivec3{0, 1, 0}].load_buffers();
                        }
                        if(chunk_i.z == 0) {
                            core.chunks[i + glm::ivec3{0, 0, -1}].load_buffers();
                        } else if(chunk_i.z == 15) {
                            core.chunks[i + glm::ivec3{0, 0, 1}].load_buffers();
                        }
                    }
                }
                break;
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
        core.up_dir = glm::rotate(identity_matrix_4, float(2 * M_PI * (delta_time / 3)), core.view_dir) * glm::vec4(core.up_dir, 1.0);
    }
    if(core.key_map[GLFW_KEY_Q]) {
        core.up_dir = glm::rotate(identity_matrix_4, float(2 * M_PI * -(delta_time / 3)), core.view_dir) * glm::vec4(core.up_dir, 1.0);
    }

    glm::ivec3 new_coord = core.view_pos;
    if(core.current_coordinate != new_coord) {
        core.current_coordinate = new_coord;
        std::get<1>(core.gui_core.widgets[1]).load_buffers(core.gui_core.font, "Coordinates: " + std::to_string((int)core.current_coordinate.x) + " " + std::to_string((int)core.current_coordinate.y) + " " + std::to_string((int)core.current_coordinate.z));
    }

    glm::ivec3 new_index = glm::floor(core.view_pos / 16.0f);
    if(core.current_index != new_index) {
        core.current_index = new_index;
        std::get<1>(core.gui_core.widgets[2]).load_buffers(core.gui_core.font, "Chunk: " + std::to_string(core.current_index.x) + " " + std::to_string(core.current_index.y) + " " + std::to_string(core.current_index.z));
        for(int x = -region.x; x <= region.x; ++x) {
            for(int y = -region.y; y <= region.y; ++y) {
                for(int z = -region.z; z <= region.z; ++z) {
                    glm::ivec3 key = new_index + glm::ivec3{x, y, z};
                    if(!core.chunks.contains(key)) {
                        core.chunks.insert({key, Chunk(key)});
                        core.chunks[key].generate();
                    }
                    if(!core.chunks[key].buffer.initialized) core.chunks[key].load_buffers();
                    //std::cout << key.x << " " << key.y << " " << key.z << "\n";
                }
            }
        }

        std::cout << new_index.x << " " << new_index.y << " " << new_index.z << "\n";
    }

    core.tick_time += delta_time;

    if(core.tick_time > 0x0.4p0) {
        core.tick_time = 0.0;

        auto set = std::move(core.block_updates);
        core.block_updates.clear();

        for(glm::ivec3 i : set) {
            block_update(i);
        }

        for(glm::ivec3 i : core.changed_chunks) {
            core.chunks[i].load_buffers();
        }
    }

    core.frame_time += delta_time;
    ++core.frame_count;

    if(core.frame_count >= 0x100) {
        std::get<1>(core.gui_core.widgets[0]).load_buffers(core.gui_core.font, "FPS: " + std::to_string((int)(core.frame_count * (1.0 / core.frame_time))));
        core.frame_count = 0;
        core.frame_time = 0.0;
    }

    core.gui_core.events.clear();
}

void Core::game_loop() {
    glEnable(GL_DEPTH_TEST);

    uint64_t current_time = get_time();
    double delta_time = double(current_time - time) / 1000000 * (86400 / 0x10000);
    time = current_time;

    calculate(delta_time);



    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gui_framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    glm::mat4 view = glm::lookAt(view_pos, view_pos + view_dir, up_dir);
    glm::mat4 projection = glm::perspective(float(2 * M_PI * 0x0.4p0), float(screen_size.x) / screen_size.y, 0x0.1p0f, 0x100.0p0f);
    
    chunk_shader.use();
    for(int x = current_index.x - region.x; x <= current_index.x + region.x; ++x) {
        for(int y = current_index.y - region.y; y <= current_index.y + region.y; ++y) {
            for(int z = current_index.z - region.z; z <= current_index.z + region.z; ++z) {
                glm::ivec3 key = {x, y, z};
                Chunk& c = chunks[key];
                if(c.buffer.initialized) {
                    c.buffer.bind();
                    glUniformMatrix4fv(0, 1, false, &view[0][0]);
                    glUniformMatrix4fv(1, 1, false, &projection[0][0]);
                    glUniform3f(2, key.x, key.y, key.z);
                    glDrawArrays(GL_TRIANGLES, 0, c.buffer.vertices);
                }
            }
        }
    }

    grid_shader.use();
    glUniformMatrix4fv(0, 1, false, &view[0][0]);
    glUniformMatrix4fv(1, 1, false, &projection[0][0]);
    glUniform1f(2, 0x0.1p0f);
    glUniform1f(3, 0x100.0p0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

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

    glDisable(GL_DEPTH_TEST);

    gui_framebuffer.bind();

    gui_core.draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    screen_shader.use();
    gui_framebuffer.color_tex[0].bind(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
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
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    core.init();

    core.time = get_time();
    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}