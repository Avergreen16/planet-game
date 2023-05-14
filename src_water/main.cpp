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

std::vector<glm::vec3> v;

aabb hb = {glm::vec3(-0.375, -0.375, -1.375) * 2.0f, glm::vec3(0.75, 0.75, 1.75) * 2.0f};
obb camera_hitbox;

glm::vec3 pos = glm::vec3(-9, 4, 5);
glm::vec3 vel = {0, 0, 0};
aabb box = {glm::vec3(-0.375, -0.375, -1.0) * 2.0f, glm::vec3(0.75, 0.75, 1.75) * 2.0f};
obb box_hitbox;

bool is_collision = false;

float bounce = 0.0f;
float friction = 0.5f;

struct col_queue_compare {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        glm::vec3 diff_a = a - core.view_pos;
        glm::vec3 diff_b = b - core.view_pos;
        float sum_a = abs(diff_a.x) + abs(diff_a.y) + abs(diff_a.z);
        float sum_b = abs(diff_b.x) + abs(diff_b.y) + abs(diff_b.z);
        return sum_a > sum_b;
    }
};

struct col_queue_compare1 {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        glm::vec3 diff_a = a - pos;
        glm::vec3 diff_b = b - pos;
        float sum_a = abs(diff_a.x) + abs(diff_a.y) + abs(diff_a.z);
        float sum_b = abs(diff_b.x) + abs(diff_b.y) + abs(diff_b.z);
        return sum_a > sum_b;
    }
};

std::priority_queue<glm::vec3, std::vector<glm::vec3>, col_queue_compare> collision_queue;
std::priority_queue<glm::vec3, std::vector<glm::vec3>, col_queue_compare1> collision_queue_box;

bool collision_test() {
    glm::vec3 mtv = {0, 0, 0};

    for(int i = 0; i < 8; ++i) {
        glm::vec3 iv = {i & 1, (i >> 1) & 1, i >> 2};
        camera_hitbox.vertices[i] = /*view */ glm::vec4(hb.pos + hb.size * iv + core.view_pos, 1.0);
    }

    for(int i = 0; i < 8; ++i) {
        glm::vec3 iv = {i & 1, (i >> 1) & 1, i >> 2};
        box_hitbox.vertices[i] = glm::vec4(box.pos + box.size * iv + pos, 1.0);
    }

    if(check_collisions(box_hitbox, camera_hitbox, mtv)) {
        glm::vec3 impact_vel = vel - core.vel;
        glm::vec3 collision_normal = glm::normalize(mtv);
        
        glm::vec3 penetration = collision_normal * glm::dot(impact_vel, collision_normal);
        glm::vec3 tangent = impact_vel - penetration;

        float r = 1 + bounce;
        float f = friction;

        float coulomb = -glm::dot(collision_normal, glm::normalize(impact_vel)) * glm::length(impact_vel) * 54;

        glm::vec3 new_vel = penetration * r + ((glm::isnan(coulomb)) ? glm::vec3(0) : tangent * ((abs(f) < abs(coulomb) || glm::isnan(coulomb)) ? f : coulomb));
        vel -= new_vel * 0.5f;
        core.vel += new_vel * 0.5f;

        pos += mtv * 0.5f;
        core.view_pos -= mtv * 0.5f;
        for(glm::vec3& v : camera_hitbox.vertices) v += mtv;
    }

    collision_queue = std::priority_queue<glm::vec3, std::vector<glm::vec3>, col_queue_compare>();
    v.clear();

    glm::mat4 view = glm::inverse(glm::lookAt({0, 0, 0}, core.view_dir, core.up_dir));
    glm::vec4 yv = view[1];
    glm::vec4 zv = view[2];
    view[1] = zv;
    view[2] = yv;

    bool collide = false;

    std::array<std::array<int, 2>, 3> minmax;

    minmax = {
        std::array<int, 2>{__INT_MAX__, -__INT_MAX__},
        std::array<int, 2>{__INT_MAX__, -__INT_MAX__},
        std::array<int, 2>{__INT_MAX__, -__INT_MAX__}
    };

    for(int i = 0; i < 3; ++i) {
        for(glm::vec3 v : camera_hitbox.vertices) {
            minmax[i][0] = std::min(minmax[i][0], (int)floor(v[i] - 0.5f));
            minmax[i][1] = std::max(minmax[i][1], (int)ceil(v[i] - 0.5f));
        }
    }

    for(int x = minmax[0][0]; x <= minmax[0][1]; ++x) {
        for(int y = minmax[1][0]; y <= minmax[1][1]; ++y) {
            for(int z = minmax[2][0]; z <= minmax[2][1]; ++z) {
                collision_queue.push(glm::vec3{x, y, z});
            }
        }
    }

    while(collision_queue.size() != 0) {
        glm::vec3 vec = collision_queue.top();
        collision_queue.pop();
        if(get_voxel(vec) != 0) {
            v.push_back(vec);
            obb _obb;

            for(int x = 0; x < 2; ++x) {
                for(int y = 0; y < 2; ++y) {
                    for(int z = 0; z < 2; ++z) {
                        uint8_t index = x + y * 2 + z * 4;
                        _obb.vertices[index] = (corners[index] + vec);
                    }
                }
            }

            if(check_collisions(camera_hitbox, _obb, mtv)) {
                glm::vec3 impact_vel = core.vel;
                glm::vec3 collision_normal = glm::normalize(mtv);
                
                glm::vec3 penetration = collision_normal * glm::dot(impact_vel, collision_normal);
                glm::vec3 tangent = impact_vel - penetration;

                float r = 1 + bounce;
                float f = friction;

                float coulomb = -glm::dot(collision_normal, glm::normalize(impact_vel)) * glm::length(impact_vel) * 54;

                core.vel -= penetration * r + ((glm::isnan(coulomb)) ? glm::vec3(0) : tangent * ((abs(f) < abs(coulomb)) ? f : coulomb));

                core.view_pos += mtv;
                for(glm::vec3& v : camera_hitbox.vertices) v += mtv;
                collide = true;
            }
        }
    }




    collision_queue_box = std::priority_queue<glm::vec3, std::vector<glm::vec3>, col_queue_compare1>();

    glm::vec3 center = glm::round(core.view_pos);

    minmax = {
        std::array<int, 2>{__INT_MAX__, -__INT_MAX__},
        std::array<int, 2>{__INT_MAX__, -__INT_MAX__},
        std::array<int, 2>{__INT_MAX__, -__INT_MAX__}
    };

    for(int i = 0; i < 3; ++i) {
        for(glm::vec3 v : box_hitbox.vertices) {
            minmax[i][0] = std::min(minmax[i][0], (int)floor(v[i] - 0.5f));
            minmax[i][1] = std::max(minmax[i][1], (int)ceil(v[i] - 0.5f));
        }
    }

    for(int x = minmax[0][0]; x <= minmax[0][1]; ++x) {
        for(int y = minmax[1][0]; y <= minmax[1][1]; ++y) {
            for(int z = minmax[2][0]; z <= minmax[2][1]; ++z) {
                collision_queue_box.push(glm::vec3{x, y, z});
            }
        }
    }

    while(collision_queue_box.size() != 0) {
        glm::vec3 vec = collision_queue_box.top();
        collision_queue_box.pop();
        if(get_voxel(vec) != 0) {
            v.push_back(vec);
            obb _obb;

            for(int x = 0; x < 2; ++x) {
                for(int y = 0; y < 2; ++y) {
                    for(int z = 0; z < 2; ++z) {
                        uint8_t index = x + y * 2 + z * 4;
                        _obb.vertices[index] = (corners[index] + vec);
                    }
                }
            }

            if(check_collisions(box_hitbox, _obb, mtv)) {
                glm::vec3 impact_vel = vel;
                glm::vec3 collision_normal = glm::normalize(mtv);
                
                glm::vec3 penetration = collision_normal * glm::dot(impact_vel, collision_normal);
                glm::vec3 tangent = impact_vel - penetration;

                float r = 1 + bounce;
                float f = friction;

                float coulomb = -glm::dot(collision_normal, glm::normalize(impact_vel)) * glm::length(impact_vel) * 54;

                vel -= penetration * r + ((glm::isnan(coulomb)) ? glm::vec3(0) : tangent * ((abs(f) < abs(coulomb)) ? f : coulomb));

                pos += mtv;
                for(glm::vec3& v : box_hitbox.vertices) v += mtv;
            }
        }
    }

    return collide;
}

float coyote_time = 0.0f;

void calculate(double delta_time) {
    bool jump = false;

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
                        --core.active_voxel;
                    }
                } else if(k.key == GLFW_KEY_EQUAL) {
                    if(k.action == GLFW_PRESS) {
                        ++core.active_voxel;
                    }
                } else if(k.key == GLFW_KEY_SPACE) {
                    if(k.action == GLFW_PRESS) {
                        jump = true;
                    }
                } else if(k.key == GLFW_KEY_C) {
                    if(k.action == GLFW_PRESS) {
                        core.chunk_debug = !core.chunk_debug;
                    }
                }
                break;
            }
            case 1: {
                mouse_button_event& m = std::get<1>(e);

                if(m.button == GLFW_MOUSE_BUTTON_LEFT) {
                    if(m.action == GLFW_PRESS) {
                        glm::ivec3 pos;
                        if(raycast(core.view_pos, core.view_dir, pos, 10.0f)) {
                            get_voxel(pos) = 0;

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
                        if(raycast_place(core.view_pos, core.view_dir, pos, 10.0f)) {
                            uint16_t& block = get_voxel(pos);

                            glm::ivec3 key = glm::floor((glm::vec3)pos / 32.0f);

                            if(block == 0) {
                                block = core.active_voxel;
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
        core.view_pos += glm::vec3(glm::normalize(core.view_dir) * float(core.move_speed * delta_time));
    }
    if(core.key_map[GLFW_KEY_S]) {
        core.view_pos -= glm::vec3(glm::normalize(core.view_dir) * float(core.move_speed * delta_time));
    }
    glm::vec3 sideways = glm::normalize(glm::cross(core.view_dir, core.up_dir));
    if(core.key_map[GLFW_KEY_D]) {
        core.view_pos += glm::vec3(glm::normalize(sideways) * float(core.move_speed * delta_time));
    }
    if(core.key_map[GLFW_KEY_A]) {
        core.view_pos -= glm::vec3(glm::normalize(sideways) * float(core.move_speed * delta_time));
    }
    if(core.key_map[GLFW_KEY_LEFT_SHIFT]) {
        core.view_pos -= core.up_dir * float(core.move_speed * delta_time);
    }
    if(core.key_map[GLFW_KEY_SPACE]) {
        core.view_pos += core.up_dir * float(core.move_speed * delta_time);
    }

    //core.vel += core.accel * float(delta_time);
    core.view_pos += core.vel * float(delta_time);

    vel += core.accel * float(delta_time);
    pos += vel * float(delta_time);
    
    core.chunk_allocate_thread_mutex.lock();

    is_collision = collision_test();
    if(is_collision) {
        coyote_time = 0.0f;
    } else {
        coyote_time += delta_time;
    }

    /*if(jump && coyote_time < 0.2f) {
        core.vel.z += 10;
    }*/

    if(raycast(core.view_pos, core.view_dir, core.selected_block, 10.0f)) {
        core.block_selected = true;
    } else {
        core.block_selected = false;
    }
    core.chunk_allocate_thread_mutex.unlock();

    glm::ivec3 new_coord = glm::floor(core.view_pos);
    if(core.current_coordinate != new_coord) {
        core.current_coordinate = new_coord;
        std::get<1>(core.gui_core.widgets[1]).load_buffers(core.gui_core.font, "Coordinates: " + to_hex((int)core.current_coordinate.x) + " " + to_hex((int)core.current_coordinate.y) + " " + to_hex((int)core.current_coordinate.z));
    }

    glm::ivec3 new_index = glm::floor(core.view_pos / 32.0f);
    if(core.current_index != new_index) {
        core.gen_queue_mutex.lock();
        core.current_index = new_index;
        std::get<1>(core.gui_core.widgets[2]).load_buffers(core.gui_core.font, "Chunk: " + to_hex(core.current_index.x) + " " + to_hex(core.current_index.y) + " " + to_hex(core.current_index.z));
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
        std::get<1>(core.gui_core.widgets[0]).load_buffers(core.gui_core.font, "FPS: " + to_hex((int)(core.frame_count * (1.0 / core.frame_time))));
        core.frame_count = 0;
        core.frame_time = 0.0;
    }

    core.gui_core.events.clear();
}

void Space_core::draw(glm::mat4 view_mat, glm::mat4 proj_mat) {
    for(Planet& p : planets) {
        float diff = (double(t - start_time) / 1000000);
        glm::mat4 rot_mat = glm::rotate(float((M_PI * 2) * diff / p.rotation), glm::vec3(0, 0, 1));
        glm::mat4 trans_mat = glm::translate(p.position);

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
        glUniformMatrix4fv(6, 1, false, &rot_mat[0][0]);

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

glm::vec3 get_perp_vec(glm::vec3 v) {
    return glm::normalize((abs(v.x) > abs(v.z)) ? glm::vec3(v.y, -v.x, 0) : glm::vec3(0, -v.z, v.y));
}

glm::vec3 multiply(glm::vec3 v, glm::mat4 m) {
    return m * glm::vec4(v, 1.0);
}

std::array<glm::ivec2, 4> character_select = {glm::ivec2{0, 2}, glm::ivec2{0, 1}, glm::ivec2{0, 0}, glm::ivec2{0, 3}};

glm::vec2 proj_plane(glm::vec3 a, glm::vec3 normal) {
    glm::vec3 perp_x = get_perp_vec(normal);
    glm::vec3 perp_y = glm::cross(normal, perp_x);

    return glm::vec2(dot(perp_x, a), dot(perp_y, a));
}

glm::vec2 proj_vec(glm::vec2 a, glm::vec2 axis) {
    glm::vec2 y_axis = {-axis.y, axis.x};
    return glm::vec2(dot(a, axis), dot(a, y_axis));
}

glm::ivec2 get_select(glm::vec3 billbd_pos, glm::vec3 view_pos, glm::vec3 billbd_look, glm::vec3 billbd_axis) {
    glm::vec2 look_proj = proj_plane(billbd_look, billbd_axis);
    glm::vec2 view_proj = proj_plane(view_pos - billbd_pos, billbd_axis);
    
    view_proj = glm::normalize(proj_vec(view_proj, look_proj));

    if(view_proj.x > M_SQRT1_2) {
        return character_select[0];
    } else if(view_proj.x < -M_SQRT1_2) {
        return character_select[2];
    } else if(view_proj.y < 0) {
        return character_select[1];
    } else {
        return character_select[3];
    }
}

void Core::game_loop() {

    glEnable(GL_DEPTH_TEST);

    uint64_t current_time = get_time();
    double delta_time = double(current_time - time) / 1000000;
    time = current_time;

    calculate(delta_time);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gui_framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    terrain_framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    const float fov = 2 * M_PI * 0x0.4p0;
    float near_plane = 0.02f;
    float far_plane = 4000.0f;
    float aspect = float(screen_size.x) / screen_size.y;

    glm::mat4 view = glm::lookAt(view_pos, view_pos + view_dir, up_dir);
    glm::mat4 projection = glm::perspective(fov, aspect, near_plane, far_plane);

    /*glm::mat4 rot_mat = glm::rotate(float((M_PI * 2) * (double(space_core.t - space_core.start_time) / 1000000) / space_core.planets[8].rotation), glm::vec3(0, 0, 1));
    
    glm::vec3 surface_vec = rot_mat * glm::vec4(glm::normalize(glm::vec3(-0.5, -0.3, -0.1)) * (space_core.planets[8].radius + 0.15f), 1.0);

    glm::vec3 look_dir = glm::vec4(glm::normalize(cross(surface_vec, glm::vec3(0.0, 0.0, 1.0))), 1.0);
    glm::mat4 space_view = glm::lookAt(space_core.planets[8].position + surface_vec, space_core.planets[8].position + surface_vec + look_dir, surface_vec);
    glm::mat4 space_projection = glm::perspective(float(2 * M_PI * 0x0.4p0), float(screen_size.x) / screen_size.y, 0x0.2p0f, 0x1800.0p0f);

    glm::vec3 rel_sun_pos = space_view * glm::vec4(space_core.planets[0].position, 1.0);

    glm::vec3 sky_col = {0.4, 0.7, 1.0};
    sky_col *= std::max(0.0f, glm::dot(normalize(rel_sun_pos), glm::vec3(0, 1, 0)));
    space_core.framebuffer.bind();
    glClearColor(sky_col.x, sky_col.y, sky_col.z, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    space_view = (glm::lookAt(glm::vec3(0, 0, 0), view_dir, up_dir) * glm::inverse(glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1)))) * space_view;

    space_core.framebuffer.bind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    space_core.framebuffer.color_tex[0].bind(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    screen_shader.use();
    glDrawArrays(GL_TRIANGLES, 0, 6);*/
    
    space_core.draw(view, projection);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    chunk_shader.use();
    tex.bind(0);

    glm::vec3 light_dir = normalize(glm::vec3(0.0, 0.0, 1.0));

    float tanhalf = tan(fov * 0.5);
    float h_near = tanhalf * near_plane;
    float w_near = h_near * aspect;
    float h_far = tanhalf * far_plane;
    float w_far = h_far * aspect;
    glm::mat4 inverse_view = glm::inverse(view);
    hexahedron frustum({multiply({-w_near, -h_near, -near_plane}, inverse_view), multiply({w_near, -h_near, -near_plane}, inverse_view), multiply({-w_far, -h_far, -far_plane}, inverse_view), multiply({w_far, -h_far, -far_plane}, inverse_view), multiply({-w_near, h_near, -near_plane}, inverse_view), multiply({w_near, h_near, -near_plane}, inverse_view), multiply({-w_far, h_far, -far_plane}, inverse_view), multiply({w_far, h_far, -far_plane}, inverse_view)});

    chunk_allocate_thread_mutex.lock();
    for(int x = current_index.x - region.x; x <= current_index.x + region.x; ++x) {
        for(int y = current_index.y - region.y; y <= current_index.y + region.y; ++y) {
            for(int z = current_index.z - region.z; z <= current_index.z + region.z; ++z) {
                glm::ivec3 key = {x, y, z};
                if(chunks.contains(key)) {
                    Chunk& c = chunks[key];
                    if(c.status == 2 || c.status == 3) {
                        obb chunk_obb(glm::vec3(key * 0x20), {0x20, 0x20, 0x20});
                        if(check_collisions(chunk_obb, frustum)) {
                            c.buffer.bind();
                            glUniformMatrix4fv(0, 1, false, &view[0][0]);
                            glUniformMatrix4fv(1, 1, false, &projection[0][0]);
                            glUniform3f(2, key.x, key.y, key.z);
                            glUniform3fv(3, 1, &light_dir[0]);
                            glDrawArrays(GL_TRIANGLES, 0, c.buffer.vertices);
                        }
                    }
                }
            }
        }
    }

    /*core.any_shader.use();
    glUniformMatrix4fv(0, 1, false, &projection[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniform3fv(2, 8, &camera_hitbox.vertices[0][0]);
    glDrawArrays(GL_LINES, 0, 24);

    core.any_shader.use();
    glUniformMatrix4fv(0, 1, false, &projection[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniform3fv(2, 8, &box_hitbox.vertices[0][0]);
    glDrawArrays(GL_LINES, 0, 24);*/
    glm::vec3 axis = {0, 0, 1};

    glm::vec3 diff = core.view_pos - pos;
    glm::mat4 model = glm::inverse(glm::lookAt(pos, diff - axis * dot(diff, axis) + pos, axis));

    x_tex.bind(0);
    core.billboard_shader.use();
    glUniformMatrix4fv(0, 1, false, &projection[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform2f(3, 4, 4);

    glm::vec2 active_tex = get_select(pos, view_pos, {0, -1, 0}, axis);
    glm::vec4 select = {active_tex.x, active_tex.y, 5, 4};
    glUniform4fv(4, 1, &select[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /*any_shader.use();
    glUniformMatrix4fv(0, 1, false, &projection[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniform3fv(2, 8, &frustum.vertices[0][0]);
    glDrawArrays(GL_LINES, 0, 24);*/

    chunk_allocate_thread_mutex.unlock();

    if(chunk_debug) {
        glm::vec3 chunk_vec = core.current_index * 0x20;

        chunk_debug_shader.use();
        glUniformMatrix4fv(0, 1, false, &projection[0][0]);
        glUniformMatrix4fv(1, 1, false, &view[0][0]);
        glUniform3fv(2, 1, &chunk_vec[0]);
        glDrawArrays(GL_LINES, 0, 792);
    }

    /*for(glm::vec3 vec3 : v) {
        cube_shader.use();
        glUniformMatrix4fv(0, 1, false, &view[0][0]);
        glUniformMatrix4fv(1, 1, false, &projection[0][0]);
        glUniform3fv(2, 1, &vec3[0]);
        glUniform3f(3, 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 24);
    }*/

    if(core.block_selected) {
        cube_shader.use();
        glm::vec3 fv = core.selected_block;
        glUniformMatrix4fv(0, 1, false, &view[0][0]);
        glUniformMatrix4fv(1, 1, false, &projection[0][0]);
        glUniform3fv(2, 1, &fv[0]);
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

    glDisable(GL_DEPTH_TEST);
    
    atmo_shader.use();
    terrain_framebuffer.depth_tex.bind(0);
    terrain_framebuffer.color_tex[0].bind(1);
    glUniformMatrix4fv(0, 1, false, &projection[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniform3fv(2, 1, &space_core.planets[0].position[0]);
    glUniform1f(3, 1.0 / 0.9);
    glUniform3fv(4, 1, &view_pos[0]);
    glUniform2f(5, viewport_size.x, viewport_size.y);

    glm::vec3 sun_dir = glm::normalize(space_core.planets[1].position);

    glUniform3fv(6, 1, &sun_dir[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    gui_framebuffer.bind();

    gui_core.draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    screen_shader.use();

    terrain_framebuffer.color_tex[0].bind(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

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
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    core.init();

    calc_normals();

    core.time = get_time();
    while(core.game_running) {
        glfwPollEvents();

        core.game_loop();

        core.game_running = !glfwWindowShouldClose(core.window);
    }
}