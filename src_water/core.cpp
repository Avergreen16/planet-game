#pragma once
#include "stuff.cpp"
#include "gui\gui.cpp"

const glm::ivec3 region = {2, 2, 2};

glm::vec3 apply_matrix(glm::vec3& vec, glm::mat4& mat) {
    return (mat * glm::vec4{vec, 1.0}).xyz();
}

bool comparator(const glm::ivec3& a, const glm::ivec3& b) {
    return a.x > b.x || (a.x == b.x && (a.y > b.y || (a.y == b.y && a.z < b.z))); 
}

struct chunk_vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

int mod(int x, int y) {
    return x - y * floor((float)x / y);
}

glm::ivec3 mod(glm::ivec3 x, int y) {
    return {x.x - y * floor((float)x.x / y), x.y - y * floor((float)x.y / y), x.z - y * floor((float)x.z / y)};
}

std::unordered_map<uint16_t, glm::vec3> tile_colors = {
    {1, {0.5, 0.5, 0.5}},
    {2, {0.5, 0.5, 0.6}},
    {3, {0.0, 0.0, 1.0}},
    {4, {0.3, 0.3, 1.0}},
    {5, {0.5, 0.1, 1.0}},
    {6, {0.3, 0.3, 0.3}}
};

struct Chunk {
    glm::ivec3 index;
    std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x10>, 0x10>, 0x10>> data = std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x10>, 0x10>, 0x10>>(new std::array<std::array<std::array<uint16_t, 0x10>, 0x10>, 0x10>);
    Buffer buffer;
    uint8_t status = 0;

    void generate();

    void load_neighbors();

    void load_buffers();

    Chunk(glm::ivec3 i) {
        index = i;
    }
    Chunk() = default;
    Chunk(Chunk&& c) noexcept = default;
};

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};
    //glm::ivec2 monitor_size = {0, 0};
    glm::ivec2 cursor_pos = {0, 0};

    glm::mat3 screen_matrix;

    glm::vec3 view_pos = {0, 0, 0};
    glm::vec3 view_dir = {0, 1, 0};
    glm::vec3 up_dir = {0, 0, 1};
    double move_speed = 8;

    Shader grid_shader;
    Shader chunk_shader;
    Shader screen_shader;

    uint64_t time;
    double frame_time;
    double tick_time;
    uint16_t frame_count = 0;

    Gui_core gui_core;
    Framebuffer<1> gui_framebuffer;

    std::unordered_map<GLuint, bool> key_map = {
        {GLFW_KEY_RIGHT_SHIFT, false},
        {GLFW_KEY_W, false},
        {GLFW_KEY_S, false},
        {GLFW_KEY_A, false},
        {GLFW_KEY_D, false},
        {GLFW_KEY_Q, false},
        {GLFW_KEY_E, false},
        {GLFW_KEY_MINUS, false},
        {GLFW_KEY_EQUAL, false}
    };

    std::unordered_map<GLuint, bool> mouse_button_map = {
        {GLFW_MOUSE_BUTTON_LEFT, false},
        {GLFW_MOUSE_BUTTON_RIGHT, false}
    };

    std::map<glm::ivec3, Chunk, decltype(&comparator)> chunks = std::map<glm::ivec3, Chunk, decltype(&comparator)>(comparator);
    std::set<glm::ivec3, decltype(&comparator)> block_updates = std::set<glm::ivec3, decltype(&comparator)>(comparator);
    std::set<glm::ivec3, decltype(&comparator)> changed_chunks = std::set<glm::ivec3, decltype(&comparator)>(comparator);

    siv::PerlinNoise noise;
    glm::ivec3 current_index = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
    glm::ivec3 current_coordinate = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

    void game_loop();

    void init() {
        screen_matrix = glm::translate(glm::inverse(glm::scale(identity_matrix_3, glm::vec2(viewport_size / 2))), glm::vec2(-viewport_size / 2));
        gui_framebuffer.init(viewport_size.x, viewport_size.y);

        grid_shader.compile("res/shaders/grid_3d.vs", "res/shaders/grid_3d_xy.fs");
        chunk_shader.compile("res/shaders/chunk_3d.vs", "res/shaders/chunk_3d.fs");
        screen_shader.compile("res/shaders/screen.vs", "res/shaders/screen.fs");

        gui_core.init();
    }

    void char_update(unsigned int codepoint) {
        if((codepoint & 0xFF) == codepoint) gui_core.events.push_back(char_event{(uint8_t)codepoint});
    }

    void framebuffer_update(int width, int height) {
        screen_size.x = width;
        screen_size.y = height;
        viewport_size.x = width + 1 * (width & 1);
        viewport_size.y = height + 1 * (height & 1);

        glViewport(0, 0, viewport_size.x, viewport_size.y);
        gui_framebuffer.resize(viewport_size);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        std::get<1>(gui_core.widgets[3]).box.position = viewport_size / 2 + glm::ivec2{-5, -9};

        screen_matrix = glm::translate(glm::scale(identity_matrix_3, glm::vec2(2.0 / viewport_size.x, 2.0 / viewport_size.y)), glm::vec2(-viewport_size / 2));
        
        game_loop();
    }

    void cursor_pos_update(double xpos, double ypos) {
        glm::ivec2 new_cursor_pos = {xpos, screen_size.y - ypos - 1};
        glm::dvec2 difference = new_cursor_pos - cursor_pos;

        if(mouse_button_map[GLFW_MOUSE_BUTTON_RIGHT]) {
            glm::mat4 rotate_matrix_x = glm::rotate(identity_matrix_4, float(2 * M_PI * 0x0.01p0 * -difference.x), up_dir);
            glm::mat4 rotate_matrix_y = glm::rotate(identity_matrix_4, float(2 * M_PI * 0x0.01p0 * difference.y), glm::cross(view_dir, up_dir));

            view_dir = (rotate_matrix_x * (rotate_matrix_y * glm::vec4(view_dir, 1.0))).xyz();
            up_dir = rotate_matrix_y * glm::vec4(up_dir, 1.0);
        }

        cursor_pos = new_cursor_pos;
    }

    void mouse_button_update(int button, int action) {
        gui_core.events.push_back(mouse_button_event{button, action});
        if(mouse_button_map.contains(button)) {
            if(action == GLFW_PRESS) mouse_button_map[button] = true;
            else if(action == GLFW_RELEASE) mouse_button_map[button] = false;
        }
    }

    void key_update(int key, int action) {
        gui_core.events.push_back(key_event{key, action});

        if(key_map.contains(key)) {
            if(action == GLFW_PRESS) key_map[key] = true;
            else if(action == GLFW_RELEASE) key_map[key] = false;
        }
    }

    void scroll_update(double yoffset) {
        if(yoffset > 0) {
            move_speed *= 2;
        } else if(yoffset < 0) {
            move_speed /= 2;
        }
    }
};

Core core;

float get_noise(siv::PerlinNoise& noise, int octaves, glm::vec3 pos) {
    float collector = 0;
    for(int i = 0; i < octaves; i++) {
        double pow_2 = 1.0 / pow(2, i);
        collector += noise.noise3D(pos.x * pow_2, pos.y * pow_2, pos.z * pow_2) * pow_2;
    }
    return collector;
}

void Chunk::generate() {
    for(float x = 0; x < 0x10; ++x) {
        for(float y = 0; y < 0x10; ++y) {
            for(float z = 0; z < 0x10; ++z) {
                (*data.get())[x][y][z] = (get_noise(core.noise, 5, glm::vec3((index.x + x / 16) / 2, (index.y + y / 16) / 2, (index.z + z / 16) / 2)) > 0) ? ((rand() > 16384) ? 1 : 2) : 0;
            }
        }
    }
}

void Chunk::load_neighbors() {
    if(!core.chunks.contains(index + glm::ivec3{-1, 0, 0})) {
        core.chunks.insert({index + glm::ivec3{-1, 0, 0}, Chunk(index + glm::ivec3{-1, 0, 0})});
        core.chunks[index + glm::ivec3{-1, 0, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{1, 0, 0})) {
        core.chunks.insert({index + glm::ivec3{1, 0, 0}, Chunk(index + glm::ivec3{1, 0, 0})});
        core.chunks[index + glm::ivec3{1, 0, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, -1, 0})) {
        core.chunks.insert({index + glm::ivec3{0, -1, 0}, Chunk(index + glm::ivec3{0, -1, 0})});
        core.chunks[index + glm::ivec3{0, -1, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 1, 0})) {
        core.chunks.insert({index + glm::ivec3{0, 1, 0}, Chunk(index + glm::ivec3{0, 1, 0})});
        core.chunks[index + glm::ivec3{0, 1, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 0, -1})) {
        core.chunks.insert({index + glm::ivec3{0, 0, -1}, Chunk(index + glm::ivec3{0, 0, -1})});
        core.chunks[index + glm::ivec3{0, 0, -1}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 0, 1})) {
        core.chunks.insert({index + glm::ivec3{0, 0, 1}, Chunk(index + glm::ivec3{0, 0, 1})});
        core.chunks[index + glm::ivec3{0, 0, 1}].generate();
    }
}

void Chunk::load_buffers() {
    if(!buffer.initialized) buffer.init();

    load_neighbors();

    Chunk& npx = core.chunks[index + glm::ivec3{1, 0, 0}];
    Chunk& npy = core.chunks[index + glm::ivec3{0, 1, 0}];
    Chunk& npz = core.chunks[index + glm::ivec3{0, 0, 1}];

    std::vector<chunk_vertex> vertices;

    for(uint8_t x = 0; x < 0x10; ++x) {
        for(uint8_t y = 0; y < 0x10; ++y) {
            for(uint8_t z = 0; z < 0x10; ++z) {
                uint16_t id = (*data.get())[x][y][z];
                glm::vec3 pos = {x, y, z};
                if(id != 0) {
                    glm::vec3 col = tile_colors[id];
                    glm::vec3 xcol = col - glm::vec3{0.05, 0.05, 0.05};
                    glm::vec3 ycol = col - glm::vec3{0.1, 0.1, 0.1};
                
                    if(x == 0xF) {
                        if((*npx.data.get())[0][y][z] == 0) {
                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                        }
                    } else {
                        if((*data.get())[x + 1][y][z] == 0) {
                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                        }
                    }
                    
                    if(y == 0xF) {
                        if((*npy.data.get())[x][0][z] == 0) {
                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, ycol});
                        }
                    } else {
                        if((*data.get())[x][y + 1][z] == 0) {
                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, ycol});
                        }
                    }

                    if(z == 0xF) {
                        if((*npz.data.get())[x][y][0] == 0) {
                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                        }
                    } else {
                        if((*data.get())[x][y][z + 1] == 0) {
                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                        }
                    }
                } else {
                    if(x != 0xF) {
                        if((*data.get())[x + 1][y][z] != 0) {
                            glm::vec3 xcol = tile_colors[(*data.get())[x + 1][y][z]] - glm::vec3{0.05, 0.05, 0.05};

                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, xcol});
                        }
                    } else {
                        if((*npx.data.get())[0][y][z] != 0) {
                            glm::vec3 xcol = tile_colors[(*npx.data.get())[0][y][z]] - glm::vec3{0.05, 0.05, 0.05};

                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 0, 0}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, xcol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, xcol});
                        }
                    }

                    if(y != 0xF) {
                        if((*data.get())[x][y + 1][z] != 0) {
                            glm::vec3 ycol = tile_colors[(*data.get())[x][y + 1][z]] - glm::vec3{0.1, 0.1, 0.1};

                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                        }
                    } else {
                        if((*npy.data.get())[x][0][z] != 0) {
                            glm::vec3 ycol = tile_colors[(*npy.data.get())[x][0][z]] - glm::vec3{0.1, 0.1, 0.1};

                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, ycol});
                            vertices.push_back({pos + glm::vec3{0, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 0}, ycol});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, ycol});
                        }
                    }

                    if(z != 0xF) {
                        if((*data.get())[x][y][z + 1] != 0) {
                            glm::vec3 col = tile_colors[(*data.get())[x][y][z + 1]];

                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, col});
                        }
                    } else {
                        if((*npz.data.get())[x][y][0] != 0) {
                            glm::vec3 col = tile_colors[(*npz.data.get())[x][y][0]];

                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{0, 0, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 1, 1}, col});
                            vertices.push_back({pos + glm::vec3{1, 0, 1}, col});
                        }
                    }
                }
            }
        }
    }

    buffer.set_data(vertices.data(), vertices.size(), sizeof(vertices[0]));
    buffer.set_attrib(0, 3, sizeof(float) * 6, 0);
    buffer.set_attrib(1, 3, sizeof(float) * 6, sizeof(float) * 3);
}

uint16_t& get_block(glm::ivec3 pos) {
    return (*core.chunks[glm::floor((glm::vec3)pos / 16.0f)].data)[mod(pos.x, 16)][mod(pos.y, 16)][mod(pos.z, 16)];
}

uint16_t& get_block(glm::ivec3 chunk_pos, glm::ivec3 block_pos) {
    return (*core.chunks[chunk_pos].data)[block_pos.x][block_pos.y][block_pos.z];
}

void block_update(glm::ivec3 pos) {
    glm::ivec3 i = glm::floor((glm::vec3)pos / 16.0f);
    glm::ivec3 block_i = mod(pos, 16);
    uint16_t& focus_block = get_block(i, block_i);
    if(focus_block == 3 || focus_block == 4 || focus_block == 5) {
        uint16_t above = get_block(pos + glm::ivec3{0, 0, 1});
        glm::ivec3 below_coord = pos + glm::ivec3{0, 0, -1};
        uint16_t& below = get_block(below_coord);
        glm::ivec3 north_coord = pos + glm::ivec3{0, 1, 0};
        uint16_t& north = get_block(north_coord);
        glm::ivec3 east_coord = pos + glm::ivec3{1, 0, 0};
        uint16_t& east = get_block(east_coord);
        glm::ivec3 south_coord = pos + glm::ivec3{0, -1, 0};
        uint16_t& south = get_block(south_coord);
        glm::ivec3 west_coord = pos + glm::ivec3{-1, 0, 0};
        uint16_t& west = get_block(west_coord);

        uint16_t below_north = get_block(north_coord + glm::ivec3{0, 0, -1});
        uint16_t below_east = get_block(east_coord + glm::ivec3{0, 0, -1});
        uint16_t below_south = get_block(south_coord + glm::ivec3{0, 0, -1});
        uint16_t below_west = get_block(west_coord + glm::ivec3{0, 0, -1});

        if(!(((focus_block == 4 && above != 0) || focus_block == 5) && below == 4)) {
            if(below == 0 || below == 1) {
                focus_block = 0;
                below = (below == 0) ? 4 : 6;
                core.changed_chunks.insert(i);
                if(block_i.x == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    i += glm::ivec3{0, 0, -1};
                    core.changed_chunks.insert(i);
                    glm::ivec3 below_i = mod(below_coord, 16);
                    if(below_i.x == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                    } else if(below_i.x == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                    }
                    if(below_i.y == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                    } else if(below_i.y == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                    }
                } else if(block_i.z == 1) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});

                if(below == 6) {
                    core.block_updates.insert(pos + glm::ivec3{-1, 0, 1});
                    core.block_updates.insert(pos + glm::ivec3{1, 0, 1});
                    core.block_updates.insert(pos + glm::ivec3{0, -1, 1});
                    core.block_updates.insert(pos + glm::ivec3{0, 1, 1});
                }
            } else if(north == 0 && below_north == 0) {
                focus_block = 0;
                north = 5;
                core.changed_chunks.insert(i);
                if(block_i.x == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 14) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                } else if(block_i.y == 15) {
                    i += glm::ivec3{0, 1, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 north_i = mod(north_coord, 16);
                    if(north_i.x == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                    } else if(north_i.x == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                    }
                    if(north_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(north_i.z == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
            } else if(east == 0 && below_east == 0) {
                focus_block = 0;
                east = 5;
                core.changed_chunks.insert(i);
                
                if(block_i.x == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 14) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                } else if(block_i.x == 15) {
                    i += glm::ivec3{1, 0, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 east_i = mod(east_coord, 16);
                    if(east_i.y == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                    } else if(east_i.y == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                    }
                    if(east_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(east_i.z == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
            } else if(south == 0 && below_south == 0) {
                focus_block = 0;
                south = 5;
                core.changed_chunks.insert(i);
                if(block_i.x == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    i += glm::ivec3{0, -1, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 south_i = mod(south_coord, 16);
                    if(south_i.x == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                    } else if(south_i.x == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                    }
                    if(south_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(south_i.z == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                } else if(block_i.y == 1) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
            } else if(west == 0 && below_west == 0) {
                focus_block = 0;
                west = 5;
                core.changed_chunks.insert(i);

                if(block_i.x == 0) {
                    i += glm::ivec3{-1, 0, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 west_i = mod(west_coord, 16);
                    if(west_i.y == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                    } else if(west_i.y == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                    }
                    if(west_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(west_i.z == 15) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                } else if(block_i.x == 1) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 15) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
            } else if(focus_block != 3 && !(focus_block == 4 && (below == 3 || below == 4 || below == 5) || focus_block == 5 && below == 4)) {
                focus_block = 3;
                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
                //core.block_updates.insert(pos);
            }
        }
    }
}

bool raycast(glm::vec3 pos, glm::vec3 dir, glm::ivec3& output, float limit) {
    glm::vec3 dir_norm = glm::normalize(dir);

    glm::ivec3 current_voxel = glm::floor(pos);
    int step_x = (dir.x < 0) ? -1 : 1;
    int step_y = (dir.y < 0) ? -1 : 1;
    int step_z = (dir.z < 0) ? -1 : 1;

    float t_collective = 0.0;

    while(true) {
        uint16_t& block = get_block(current_voxel);
        if(block != 0) {
            output = current_voxel;
            return true;
        } else if(t_collective > limit) {
            return false;
        }

        int t_x = (float(current_voxel.x + step_x) - pos.x) / dir_norm.x;
        int t_y = (float(current_voxel.y + step_y) - pos.y) / dir_norm.y;
        int t_z = (float(current_voxel.z + step_z) - pos.z) / dir_norm.z;

        if(t_x < t_y && t_x < t_z) {
            pos += dir_norm * float(t_x);
            t_collective += t_x;
            current_voxel.x += step_x;
        } else if(t_y < t_x && t_y < t_z) {
            pos += dir_norm * float(t_y);
            t_collective += t_y;
            current_voxel.y += step_y;
        } else {
            pos += dir_norm * float(t_z);
            t_collective += t_z;
            current_voxel.z += step_z;
        }
    }


    /*glm::ivec3 current = glm::floor(pos);
    glm::vec3 step = glm::normalize(dir) / 16.0f;

    for(int i = 0; i < 96; ++i) {
        pos += step;
        glm::ivec3 new_current = glm::floor(pos);
        if(new_current != current) {
            if(get_block(new_current) != 0) {
                output = new_current;
                return true;
            }
            current = new_current;
        }
    }

    return false;*/
}

#include "gui\parameter_functions.cpp"

void init_widgets(Gui_core& self) {
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x1E}});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x3C}});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x5A}});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, core.viewport_size / 2 + glm::ivec2{-5, -9}, self.font, "+"});
}