#define USE_2D
#include "wrapper.cpp"
#include "fstream"
#include <chrono>
#include <deque>
#include <set>

time_t get_time() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

// shaders

const char* vs_grid = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

layout(location = 0) uniform mat3 matrix;
layout(location = 1) uniform vec2 cam_pos;

out vec2 world_coord;
out ivec2 recentered;

void main() {
    world_coord = (matrix * vec3(pos, 1.0)).xy;
    recentered = ivec2(0, 0);

    if(abs(cam_pos.x) > 4096.0) {
        world_coord.x -= floor(cam_pos.x / 4096.0) * 4096.0;
        recentered.x = 1;
    }
    if(abs(cam_pos.y) > 4096.0) {
        world_coord.y -= floor(cam_pos.y / 4096.0) * 4096.0;
        recentered.y = 1;
    }

    gl_Position = vec4(pos, 0.5, 1.0);
}
)""";

const char* fs_grid = R"""(
#version 460 core

layout(location = 2) uniform int scale;

in vec2 world_coord;
flat in ivec2 recentered;

out vec4 frag_color;

void main() {
    float pixel_size = 1.0 / scale;

    ivec2 floor_coord = ivec2(floor(world_coord));
    
    float pixel_shift = pixel_size * int(scale >= 16);
    ivec2 shifted_floor_coord = ivec2(floor(world_coord + pixel_shift));

    float difference_x = world_coord.x - floor_coord.x;
    float difference_y = world_coord.y - floor_coord.y;
    
    if(shifted_floor_coord.y % 16 == 0 && world_coord.y - shifted_floor_coord.y < pixel_size) {
        if(recentered.y == 0 && shifted_floor_coord.y == 0) {
            frag_color = vec4(1.0, 0.25, 0.25, 0.5);
        } else if(shifted_floor_coord.y % 256 == 0) {
            frag_color = vec4(1.0, 1.0, 0.25, 0.5);
        } else {
            if(shifted_floor_coord.x == 0 && world_coord.x - shifted_floor_coord.x < pixel_size) {
                frag_color = vec4(0.25, 0.25, 1.0, 0.5);
            } else {
                frag_color = vec4(0.25, 1.0, 1.0, 0.5);
            }
        }
    } else if(shifted_floor_coord.x % 16 == 0 && world_coord.x - shifted_floor_coord.x < pixel_size) {
        if(recentered.x == 0 && shifted_floor_coord.x == 0) {
            frag_color = vec4(0.25, 0.25, 1.0, 0.5);
        } else if(shifted_floor_coord.x % 256 == 0) {
            frag_color = vec4(1.0, 1.0, 0.25, 0.5);
        } else {
            frag_color = vec4(0.25, 1.0, 1.0, 0.5);
        }
    } else if(scale >= 16 && (difference_x < pixel_size || difference_y < pixel_size)) {
        frag_color = vec4(0.25, 1.0, 1.0, 0.25);
    } else {
        discard;
    }
}
)""";

const char* vs_hover = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

layout(location = 0) uniform mat3 pos_matrix;
layout(location = 1) uniform mat3 cam_matrix;
layout(location = 2) uniform vec4 in_tex;

out vec2 tex_coords;

void main() {
    tex_coords = pos * in_tex.zw + in_tex.xy;
    gl_Position = vec4((cam_matrix * pos_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}
)""";

const char* fs_hover = R"""(
#version 460 core
layout(location = 3) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = vec4(texelFetch(active_texture, ivec2(tex_coords), 0).rgb * 0.75 + vec3(0.25, 1.0, 1.0) * 0.25, 0.5);
}
)""";

const char* vs_select = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

layout(location = 0) uniform mat3 cam_matrix;

void main() {
    gl_Position = vec4((cam_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}
)""";

const char* fs_select = R"""(
#version 460 core
layout(location = 1) uniform float alpha;

out vec4 frag_color;

void main() {
    frag_color = vec4(0.25, 1.0, 1.0, alpha);
}
)""";

const char* vs_delete = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

layout(location = 0) uniform mat3 pos_matrix;
layout(location = 1) uniform mat3 cam_matrix;

void main() {
    gl_Position = vec4((cam_matrix * pos_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}
)""";

const char* fs_delete = R"""(
#version 460 core
layout(location = 2) uniform float alpha;

out vec4 frag_color;

void main() {
    frag_color = vec4(1.0, 0.0, 0.0, alpha);
}
)""";

const char* vs_chunk = R"""(
#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

layout(location = 0) uniform mat3 pos_matrix;
layout(location = 1) uniform mat3 cam_matrix;

out vec2 tex_coords;

void main() {
    tex_coords = tex;
    gl_Position = vec4((cam_matrix * pos_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}
)""";

const char* fs_chunk = R"""(
#version 460 core
layout(location = 2) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = texelFetch(active_texture, ivec2(tex_coords), 0);
}
)""";

const char* vs_paste = R"""(
#version 460 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

layout(location = 0) uniform mat3 pos_matrix;
layout(location = 1) uniform mat3 cam_matrix;

out vec2 tex_coords;

void main() {
    tex_coords = tex;
    gl_Position = vec4((cam_matrix * pos_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}
)""";

const char* fs_paste = R"""(
#version 460 core
layout(location = 2) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = vec4(texelFetch(active_texture, ivec2(tex_coords), 0).rgb * 0.75 + vec3(0.25, 1.0, 1.0) * 0.25, 0.5);
}
)""";

struct vec_comp {
    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        return b.y > a.y || (b.y == a.y && b.x > a.x);
    }
};

int convert(glm::ivec2 key) {
    return key.x - 256 + (key.y - 256) * 512;
}

// vertices

struct Vertex_0 {
    glm::vec2 pos;
    glm::vec2 tex;

    Vertex_0(float a, float b, float c, float d) {
        pos = {a, b};
        tex = {c, d};
    }

    Vertex_0() = default;
};

glm::vec2 screen_vertices[] = {
    glm::vec2{-1, -1},
    glm::vec2{1, -1},
    glm::vec2{-1, 1},
    glm::vec2{-1, 1},
    glm::vec2{1, -1},
    glm::vec2{1, 1}
};

Vertex_0 tile_vertices[] = {
    {0, 0, 0, 0},
    {1, 0, 16, 0},
    {0, 1, 0, 16},
    {0, 1, 0, 16},
    {1, 0, 16, 0},
    {1, 1, 16, 16}
};

struct Tile_data {
    glm::ivec2 loc;
    glm::ivec2 size;
};

struct Chunk;

enum mode{VIEW, EDIT};

enum edit_mode{SET, FILL, SELECT, PASTE};

typedef std::array<glm::ivec2, 2> tile_loc;

struct Set_action {
    tile_loc loc;
    int prev;
    int set;
};

struct Action {
    std::vector<Set_action> contents;
};

enum key_status{PRESS_ON, PRESS_OFF, UP_ON, UP_OFF};

struct Core {
    bool game_running = true;
    glm::ivec2 screen_size = {800, 600};

    glm::ivec2 mouse_tile;

    int active_tile_id = 1;
    int max_tile_id = 0;

    mode active_mode = EDIT;
    edit_mode active_edit_mode = SET;

    // used for select
    glm::ivec2 start_pos = {0, 0};
    glm::ivec2 end_pos = {0, 0};
    glm::ivec2 bottom_left = {0, 0};
    glm::ivec2 top_right = {0, 0};

    bool select_active = false;
    bool show_select = false;
    std::vector<std::vector<int>> select_data;
    Buffer select_buffer;

    bool select_drag = false;

    int tiles_in_paste;
    bool reload_paste_buffer = true;
    Buffer paste_buffer;
    glm::ivec2 paste_offset = {0, 0};

    // used for undo/redo
    Action new_action;
    std::deque<Action> action_queue;
    int pos_in_queue = -1;

    std::map<int, key_status> keymap = {
        {GLFW_KEY_LEFT_CONTROL, UP_OFF}
    };

    std::map<int, key_status> keymap_mouse = {
        {GLFW_MOUSE_BUTTON_RIGHT, UP_OFF},
        {GLFW_MOUSE_BUTTON_LEFT, UP_OFF},
        {GLFW_MOUSE_BUTTON_MIDDLE, UP_OFF}
    };

    std::vector<Tile_data> tile_data = std::vector<Tile_data>(1);
    //std::vector<Texture> textures;
    Texture texture;

    std::map<glm::ivec2, Chunk, vec_comp> chunks;

    glm::vec2 camera_pos = {0, 0};
    glm::ivec2 mouse_pos = {0, 0};
    int scale = 16;

    void insert_action() {
        if(new_action.contents.size() != 0) {
            if(action_queue.size() > pos_in_queue + 1) {
                action_queue.erase(action_queue.begin() + pos_in_queue + 1, action_queue.end());
            }
            pos_in_queue++;
            action_queue.push_back(new_action);

            new_action.contents.clear();
        }
    }

    void insert_action(Action& a) {
        if(a.contents.size() != 0) {
            if(action_queue.size() > pos_in_queue + 1) {
                action_queue.erase(action_queue.begin() + pos_in_queue + 1, action_queue.end());
            }
            pos_in_queue++;
            action_queue.push_back(a);
        }
    }

    void load_data();

    void save_game(std::string address);
};

struct Chunk {
    std::array<int, 256> tiles;
    glm::ivec2 pos;

    Buffer buffer;

    Chunk(glm::ivec2 pos) {
        tiles.fill(0);
        buffer.init();

        this->pos = pos;
    }

    Chunk() {
        tiles.fill(0);
    };

    void create_vertices(Core& core) {

        std::vector<Vertex_0> vertex_vec;

        for(int i = 0; i < 256; i++) {
            int tile = tiles[i];
            if(tile) {
                Tile_data& current_tile_data = core.tile_data[tile];
                int x = i % 16;
                int y = i / 16;

                vertex_vec.push_back(Vertex_0{x, y, current_tile_data.loc.x, current_tile_data.loc.y});
                vertex_vec.push_back(Vertex_0{x + 1, y, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y});
                vertex_vec.push_back(Vertex_0{x, y + 1, current_tile_data.loc.x, current_tile_data.loc.y + current_tile_data.size.y});
                vertex_vec.push_back(Vertex_0{x, y + 1, current_tile_data.loc.x, current_tile_data.loc.y + current_tile_data.size.y});
                vertex_vec.push_back(Vertex_0{x + 1, y, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y});
                vertex_vec.push_back(Vertex_0{x + 1, y + 1, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y + current_tile_data.size.y});
            }
        }

        buffer.bind();
        buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex_0));
        buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
        buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));
    }
};

void Core::load_data() {
    std::ifstream data;
    data.open("res\\data.txt");

    if(data.is_open()) {
        char string[64];
        bool end = false;
        while(!end) {
            data >> string;
            if(strcmp(string, "end") == 0) {
                end = true;
            } else if(strcmp(string, "tile") == 0) {
                int tile_pos = tile_data.size();
                tile_data.push_back(Tile_data());
                data >> tile_data[tile_pos].loc.x >> tile_data[tile_pos].loc.y >> tile_data[tile_pos].size.x >> tile_data[tile_pos].size.y;

                max_tile_id++;
            } else {
                end = true;
                std::cout << "error reading data.txt" << std::endl;
            }
        }
        data.close();
    } else {
        std::cout << "unable to open data.txt" << std::endl;
    }

    std::ifstream save;
    save.open("output\\save.txt");

    if(save.is_open()) {
        char string[64];
        bool end = false;
        while(!end) {
            save >> string;
            if(strcmp(string, "end") == 0) {
                end = true;
            } else if(strcmp(string, "chunk") == 0) {
                glm::ivec2 loc;
                save >> loc.x >> loc.y;

                chunks.insert({loc, Chunk(loc)});
                Chunk& chunk = chunks[loc];

                //save.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                int i = 0;
                for(; i < 256;) {
                    int id;
                    int count;
                    save >> id >> count;

                    int start = i;
                    for(; i < start + count;) {
                        chunk.tiles[i] = id;
                        i++;
                    }
                }
            } else {
                end = true;
                std::cout << "error reading data.txt" << std::endl;
            }
        }
        save.close();
    } else {
        std::cout << "unable to open save.txt" << std::endl;
    }
}

void Core::save_game(std::string address) {
    std::ofstream output;
    output.open(address);

    std::string output_string = "";
    for(auto& [key, chunk] : chunks) {
        std::string chunk_line = "chunk ";
        chunk_line += std::to_string(chunk.pos.x) + " " + std::to_string(chunk.pos.y);

        int counter = 0;
        int current_id = chunk.tiles[0];
        for(int i = 0; i < 256; i++) {
            int id = chunk.tiles[i];

            if(id != current_id) {
                chunk_line += " " + std::to_string(current_id) + " " + std::to_string(counter);
                counter = 0;
                current_id = id;
            }
            counter++;
        }
        chunk_line += " " + std::to_string(current_id) + " " + std::to_string(counter);

        chunk_line += "\n";
        if(!(current_id == 0 && counter == 256)) {
            output_string += chunk_line;
        }
    }
    output_string += "end";
    output << output_string;
    output.close();
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS) {

        if(core.keymap.contains(key)) {
            core.keymap[key] = PRESS_ON;
        } else if(key > 47 && key <= 48 + core.max_tile_id) {
            core.active_tile_id = key - 48;
        } else if(key == GLFW_KEY_F11) {
            std::vector<uint8_t> vector(core.screen_size.x * core.screen_size.y * 3);
            glReadPixels(0, 0, core.screen_size.x, core.screen_size.y, GL_RGB, GL_UNSIGNED_BYTE, vector.data());

            stbi_write_png(("output\\screenshot" + std::to_string(int(get_time() / 100000000)) + ".png").data(), core.screen_size.x, core.screen_size.y, 3, vector.data(), core.screen_size.x * 3);
        } else if(key == GLFW_KEY_ESCAPE) {
            if(core.active_mode == EDIT) {
                switch(core.active_edit_mode) {
                    case PASTE: {
                        std::set<Chunk*> edited_chunks;

                        for(int x = 0; x < core.select_data.size(); x++) {
                            for(int y = 0; y < core.select_data[0].size(); y++) {
                                int set_tile = core.select_data[x][y];
                                if(set_tile) {
                                    glm::ivec2 insert_loc(core.mouse_tile.x + core.paste_offset.x + x, core.mouse_tile.y + core.paste_offset.y + y);
                                    glm::ivec2 chunk_key = insert_loc >> 4;

                                    if(!core.chunks.contains(chunk_key)) {
                                        core.chunks.insert({chunk_key, Chunk(chunk_key)});
                                    }

                                    glm::ivec2 pos_in_chunk = insert_loc & 15;
                                    Chunk* chunk = &core.chunks[chunk_key];
                                    int& target_tile = chunk->tiles[pos_in_chunk.x + (pos_in_chunk.y << 4)];
                                    
                                    if(target_tile != set_tile) {
                                        core.new_action.contents.push_back(Set_action{{chunk_key, pos_in_chunk}, target_tile, set_tile});
                                        target_tile = set_tile;

                                        edited_chunks.insert(chunk);
                                    }
                                }
                            }
                        }

                        for(Chunk* c : edited_chunks) {
                            c->create_vertices(core);
                        }

                        core.insert_action();
                    }
                }
                core.active_edit_mode = SET;
            }
        } else if(key == GLFW_KEY_P) {
            if(core.active_mode == VIEW) {
                core.active_mode = EDIT;
            } else {
                core.active_mode = VIEW;
                if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON && core.new_action.contents.size() != 0) {
                    core.insert_action();
                }
            }
        } else if(key == GLFW_KEY_F) {
            switch(core.active_edit_mode) {
                case SET: 
                    core.active_edit_mode = FILL;
                    core.insert_action();
                    break;
                case FILL: 
                    core.active_edit_mode = SELECT;
                    break;
                case SELECT:
                    core.select_active = false;
                    core.show_select = false;
                    core.active_edit_mode = SET;
                    break;
                case PASTE: 
                    core.active_edit_mode = SET;
                    break;
            }
        } else if(key == GLFW_KEY_BACKSPACE) {
            if(core.active_mode == EDIT && core.active_edit_mode == SELECT && core.show_select) {
                Action action;

                std::set<Chunk*> edited_chunks;

                int y_dif = core.top_right.y - core.bottom_left.y + 1;

                for(int x = core.bottom_left.x; x <= core.top_right.x; x++) {
                    for(int y = core.bottom_left.y; y <= core.top_right.y; y++) {
                        glm::ivec2 chunk_key(x >> 4, y >> 4);

                        if(core.chunks.contains(chunk_key)) {
                            Chunk& chunk = core.chunks[chunk_key];
                            int pos_in_chunk = (x & 15) + ((y & 15) << 4);
                            int& tile = chunk.tiles[pos_in_chunk];
                            if(tile) {
                                action.contents.push_back(Set_action{{chunk_key, {x & 15, y & 15}}, tile, 0});
                                tile = 0;
                            }

                            edited_chunks.insert(&chunk);
                        }
                    }
                }

                core.insert_action(action);

                for(Chunk* c : edited_chunks) {
                    c->create_vertices(core);
                }

                core.show_select = false;
            }
        }

        if(core.keymap[GLFW_KEY_LEFT_CONTROL] == PRESS_ON) {
            if(key == GLFW_KEY_Z) {
                if(core.pos_in_queue != -1) {
                    Action& action = core.action_queue[core.pos_in_queue];
                    std::set<Chunk*> edited_chunks;

                    for(int i = action.contents.size() - 1; i >= 0; i--) {
                        Set_action a = action.contents[i];
                        Chunk& chunk = core.chunks[a.loc[0]];
                        chunk.tiles[a.loc[1].x + a.loc[1].y * 16] = a.prev;
                        edited_chunks.insert(&chunk);
                    }

                    for(Chunk* c : edited_chunks) {
                        c->create_vertices(core);
                    }

                    core.pos_in_queue--;
                }
            } else if(key == GLFW_KEY_Y) {
                if(core.pos_in_queue != core.action_queue.size() - 1) {
                    core.pos_in_queue++;

                    Action& action = core.action_queue[core.pos_in_queue];
                    std::set<Chunk*> edited_chunks;

                    for(int i = action.contents.size() - 1; i >= 0; i--) {
                        Set_action a = action.contents[i];
                        Chunk& chunk = core.chunks[a.loc[0]];
                        chunk.tiles[a.loc[1].x + a.loc[1].y * 16] = a.set;
                        edited_chunks.insert(&chunk);
                    }

                    for(Chunk* c : edited_chunks) {
                        c->create_vertices(core);
                    }
                }
            } else if(key == GLFW_KEY_C) {
                if(core.show_select == true) {
                    core.select_data.clear();
                    core.select_data.resize(core.top_right.x - core.bottom_left.x + 1);
                    core.tiles_in_paste = 0;

                    int y_dif = core.top_right.y - core.bottom_left.y + 1;

                    for(int x = core.bottom_left.x; x <= core.top_right.x; x++) {
                        int rel_x = x - core.bottom_left.x;
                        core.select_data[rel_x] = std::vector<int>(y_dif);
                        for(int y = core.bottom_left.y; y <= core.top_right.y; y++) {
                            glm::ivec2 chunk_key(x >> 4, y >> 4);
                            if(core.chunks.contains(chunk_key)) {
                                int tile = core.chunks[chunk_key].tiles[(x & 15) + ((y & 15) << 4)];
                                if(tile) core.tiles_in_paste++;
                                core.select_data[rel_x][y - core.bottom_left.y] = tile;
                            }
                        }
                    }

                    core.reload_paste_buffer = true;
                }
            } else if(key == GLFW_KEY_V) {
                if(core.active_mode == EDIT && core.select_data.size() != 0) {
                    switch(core.active_edit_mode) {
                        case SET: {
                            core.insert_action();
                        }
                        case SELECT: {
                            core.select_active = false;
                            core.show_select = false;
                        }
                    }
                    
                    core.active_edit_mode = PASTE;

                    if(core.reload_paste_buffer == true) {
                        std::vector<Vertex_0> vertex_vec(core.tiles_in_paste * 6);
                        int i = 0;

                        for(int x = 0; x < core.select_data.size(); x++) {
                            for(int y = 0; y < core.select_data[0].size(); y++) {
                                int tile = core.select_data[x][y];
                                if(tile) {

                                    Tile_data& current_tile_data = core.tile_data[tile];

                                    vertex_vec[i] = (Vertex_0{x, y, current_tile_data.loc.x, current_tile_data.loc.y});
                                    vertex_vec[i + 1] = (Vertex_0{x + 1, y, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y});
                                    vertex_vec[i + 2] = (Vertex_0{x, y + 1, current_tile_data.loc.x, current_tile_data.loc.y + current_tile_data.size.y});
                                    vertex_vec[i + 3] = (Vertex_0{x, y + 1, current_tile_data.loc.x, current_tile_data.loc.y + current_tile_data.size.y});
                                    vertex_vec[i + 4] = (Vertex_0{x + 1, y, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y});
                                    vertex_vec[i + 5] = (Vertex_0{x + 1, y + 1, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y + current_tile_data.size.y});
                                    i += 6;
                                }
                            }
                        }

                        core.paste_buffer.bind();
                        core.paste_buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex_0));
                        core.paste_buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
                        core.paste_buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));
                        
                        core.reload_paste_buffer = false;
                    }
                }
            } else if(key == GLFW_KEY_S) {
                core.save_game("output\\save.txt");
            }
        }

    } else if(action == GLFW_RELEASE) {
        if(core.keymap.contains(key)) {
            core.keymap[key] = UP_ON;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    glm::ivec2 new_pos(x_pos, y_pos);
    if(core.keymap_mouse[GLFW_MOUSE_BUTTON_RIGHT] == PRESS_ON) {
        glm::vec2 difference(new_pos - core.mouse_pos);
        core.camera_pos += glm::vec2(-difference.x, difference.y) / float(core.scale);
    }
    core.mouse_pos = new_pos;
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    if(y_offset > 0 && core.scale < 96) {
        if(core.scale < 16) {
            core.scale *= 2;
        } else {
            core.scale += 16;
        }
        /*core.framebuffer.resize({core.screen_size.x, core.screen_size.y});

        core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
        core.reload_active_chunks = true;*/
    } else if(y_offset < 0) {
        if(core.scale > 16) {
            core.scale -= 16;
        } else if(core.scale > 1) {
            core.scale /= 2;
        }
        /*core.framebuffer.resize({core.screen_size.x, core.screen_size.y});

        core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
        core.reload_active_chunks = true;*/
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    core.screen_size.x = width + 1 * (width % 2 == 1);
    core.screen_size.y = height + 1 * (height % 2 == 1);
    
    glViewport(0, 0, core.screen_size.x, core.screen_size.y);

    /*core.chunks_loaded = glm::ivec2(ceil((0.5 * core.screen_size.x) / (core.scale * 16)), ceil((0.5 * core.screen_size.y) / (core.scale * 16)));
    core.reload_active_chunks = true;

    core.framebuffer.resize({core.screen_size.x, core.screen_size.y});*/
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS) {
        if(core.keymap_mouse.contains(button)) {
            core.keymap_mouse[button] = PRESS_ON;
        }
    } else if(action == GLFW_RELEASE) {
        if(core.keymap_mouse.contains(button)) {
            core.keymap_mouse[button] = UP_ON;
        }
        if(button == GLFW_MOUSE_BUTTON_LEFT && core.active_mode == EDIT) {
            if(core.active_edit_mode == SET) {
                if(core.new_action.contents.size() != 0) {
                    if(core.action_queue.size() > core.pos_in_queue + 1) {
                        core.action_queue.erase(core.action_queue.begin() + core.pos_in_queue + 1, core.action_queue.end());
                    }
                    core.pos_in_queue++;
                    core.action_queue.push_back(core.new_action);

                    core.new_action.contents.clear();
                }
            } else if(core.active_edit_mode == SELECT) {
                core.select_active = false;

                if(core.start_pos == core.end_pos) {
                    core.show_select = false;
                }
            }
        }
    }
}

void modify_tile_loc(tile_loc& loc) {
    if(loc[1].x < 0) {
        loc[1].x += 16;
        loc[0].x--;
    } else if(loc[1].x > 15) {
        loc[1].x -= 16;
        loc[0].x++;
    }

    if(loc[1].y < 0) {
        loc[1].y += 16;
        loc[0].y--;
    } else if(loc[1].y > 15) {
        loc[1].y -= 16;
        loc[0].y++;
    }
}

int main() {
    Core core;

    // init glfw and set version
    if(glfwInit() == GLFW_FALSE) {
        std::cout << "ERROR: GLFW failed to load.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // create window
    GLFWwindow* window = glfwCreateWindow(core.screen_size.x, core.screen_size.y, "This is a test.", NULL, NULL);
    if(window == NULL) {
        std::cout << "ERROR: Window creation failed.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // init glad and set viewport
    if(!gladLoadGL()) {
        std::cout << "ERROR: GLAD failed to load.\n";
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, core.screen_size.x, core.screen_size.y);

    // load data
    core.load_data();

    glfwSetWindowUserPointer(window, &core);

    Buffer grid_buffer;
    grid_buffer.init();
    grid_buffer.set_data(screen_vertices, 6, sizeof(glm::vec2));
    grid_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);

    Buffer hover_buffer;
    hover_buffer.init();
    hover_buffer.set_data(tile_vertices, 6, sizeof(Vertex_0));
    hover_buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
    hover_buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));

    core.select_buffer.init();
    core.paste_buffer.init();

    Shader grid_shader;
    grid_shader.compile(vs_grid, fs_grid);

    Shader hover_shader;
    hover_shader.compile(vs_hover, fs_hover);

    Shader chunk_shader;
    chunk_shader.compile(vs_chunk, fs_chunk);

    Shader delete_shader;
    delete_shader.compile(vs_delete, fs_delete);

    Shader select_shader;
    select_shader.compile(vs_select, fs_select);

    Shader paste_shader;
    paste_shader.compile(vs_paste, fs_paste);

    // set up stbi and load textures
    stbi_set_flip_vertically_on_load(true);
    stbi__flip_vertically_on_write = true;

    core.texture.load("res\\tile.png");

    // set callbacks
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

    /*core.chunks.insert({0, Chunk({0, 0})});
    core.chunks[0].tiles[0] = 1;
    core.chunks[0].tiles[39] = 1;
    core.chunks[0].create_vertices(core);*/

    for(auto& [key, chunk] : core.chunks) {
        chunk.create_vertices(core);
    }

    time_t start_time = get_time();
    time_t time_container = get_time();

    while(core.game_running) {
        // -> logic

        time_t new_time = get_time();
        time_t delta_time = new_time - time_container;
        time_container = new_time;

        glfwPollEvents();

        // get mouse position in world
        glm::vec2 mouse_pos_world = glm::vec2(core.mouse_pos.x - core.screen_size.x / 2, -core.mouse_pos.y - 1 + core.screen_size.y / 2);
        mouse_pos_world = mouse_pos_world / float(core.scale) + core.camera_pos;
        core.mouse_tile = glm::floor(mouse_pos_world);

        if(core.active_mode == EDIT) {
            if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON) {
                switch(core.active_edit_mode) {
                    case SET: {
                        glm::ivec2 location = core.mouse_tile;

                        glm::ivec2 chunk_key = glm::floor(glm::vec2(location) / 16.0f);
                        glm::ivec2 pos_in_chunk = location - chunk_key * 16;

                        if(!core.chunks.contains(chunk_key)) {
                            core.chunks.insert({chunk_key, Chunk(chunk_key)});
                        }

                        Chunk* chunk = &core.chunks[chunk_key];
                        int& focus_tile = chunk->tiles[pos_in_chunk.y * 16 + pos_in_chunk.x];

                        if(focus_tile != core.active_tile_id) {
                            core.new_action.contents.push_back(Set_action(tile_loc{chunk_key, pos_in_chunk}, focus_tile, core.active_tile_id));

                            focus_tile = core.active_tile_id;
                            chunk->create_vertices(core);
                        }
                        break;
                    }

                    case FILL: {
                        glm::ivec2 location = core.mouse_tile;

                        glm::ivec2 chunk_key = glm::floor(glm::vec2(location) / 16.0f);
                        glm::ivec2 pos_in_chunk = location - chunk_key * 16;

                        if(!core.chunks.contains(chunk_key)) {
                            core.chunks.insert({chunk_key, Chunk(chunk_key)});
                        }

                        int focus_id = core.chunks[chunk_key].tiles[pos_in_chunk.y * 16 + pos_in_chunk.x];

                        if(focus_id != core.active_tile_id) {
                            std::set<Chunk*> edited_chunks;
                            Action action;

                            std::queue<tile_loc> queue;
                            queue.push(tile_loc{chunk_key, pos_in_chunk});
                            int counter = 0;
                            while(queue.size() != 0 && counter < 2113) {
                                tile_loc current_loc = queue.front();
                                queue.pop();
                                if(!core.chunks.contains(current_loc[0])) {
                                    core.chunks.insert({current_loc[0], Chunk(current_loc[0])});
                                }
                                Chunk* chunk = &core.chunks[current_loc[0]];

                                int& tile = chunk->tiles[current_loc[1].x + current_loc[1].y * 16];

                                if(tile == focus_id) {
                                    counter++;

                                    tile = core.active_tile_id;
                                    action.contents.push_back(Set_action(current_loc, focus_id, core.active_tile_id));

                                    tile_loc north = current_loc;
                                    tile_loc east = current_loc;
                                    tile_loc south = current_loc;
                                    tile_loc west = current_loc;

                                    north[1].y++;
                                    east[1].x++;
                                    south[1].y--;
                                    west[1].x--;

                                    modify_tile_loc(north);
                                    modify_tile_loc(east);
                                    modify_tile_loc(south);
                                    modify_tile_loc(west);

                                    queue.push(north);
                                    queue.push(east);
                                    queue.push(south);
                                    queue.push(west);
                                }

                                edited_chunks.insert(chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            core.insert_action(action);
                        }

                        core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                        break;
                    }

                    case SELECT: {
                        if(!core.select_active && core.show_select && core.mouse_tile.x >= core.bottom_left.x && core.mouse_tile.x <= core.top_right.x && core.mouse_tile.y >= core.bottom_left.y && core.mouse_tile.y <= core.top_right.y) {
                            core.select_drag = true;
                            core.paste_offset = core.bottom_left - core.mouse_tile;

                            //

                            core.select_data.clear();
                            core.select_data.resize(core.top_right.x - core.bottom_left.x + 1);
                            core.tiles_in_paste = 0;

                            int y_dif = core.top_right.y - core.bottom_left.y + 1;

                            for(int x = core.bottom_left.x; x <= core.top_right.x; x++) {
                                int rel_x = x - core.bottom_left.x;
                                core.select_data[rel_x] = std::vector<int>(y_dif);
                                for(int y = core.bottom_left.y; y <= core.top_right.y; y++) {
                                    glm::ivec2 chunk_key(x >> 4, y >> 4);
                                    if(core.chunks.contains(chunk_key)) {
                                        int tile = core.chunks[chunk_key].tiles[(x & 15) + ((y & 15) << 4)];
                                        if(tile) core.tiles_in_paste++;
                                        core.select_data[rel_x][y - core.bottom_left.y] = tile;
                                    }
                                }
                            }

                            core.reload_paste_buffer = true;

                            //

                            std::set<Chunk*> edited_chunks;

                            y_dif = core.top_right.y - core.bottom_left.y + 1;

                            for(int x = core.bottom_left.x; x <= core.top_right.x; x++) {
                                for(int y = core.bottom_left.y; y <= core.top_right.y; y++) {
                                    glm::ivec2 chunk_key(x >> 4, y >> 4);

                                    if(core.chunks.contains(chunk_key)) {
                                        Chunk& chunk = core.chunks[chunk_key];
                                        int pos_in_chunk = (x & 15) + ((y & 15) << 4);
                                        int& tile = chunk.tiles[pos_in_chunk];
                                        if(tile) {
                                            core.new_action.contents.push_back(Set_action{{chunk_key, {x & 15, y & 15}}, tile, 0});
                                            tile = 0;
                                        }

                                        edited_chunks.insert(&chunk);
                                    }
                                }
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }
                            
                            //

                            core.active_edit_mode = PASTE;

                            if(core.reload_paste_buffer == true) {
                                std::vector<Vertex_0> vertex_vec(core.tiles_in_paste * 6);
                                int i = 0;

                                for(int x = 0; x < core.select_data.size(); x++) {
                                    for(int y = 0; y < core.select_data[0].size(); y++) {
                                        int tile = core.select_data[x][y];
                                        if(tile) {

                                            Tile_data& current_tile_data = core.tile_data[tile];

                                            vertex_vec[i] = (Vertex_0{x, y, current_tile_data.loc.x, current_tile_data.loc.y});
                                            vertex_vec[i + 1] = (Vertex_0{x + 1, y, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y});
                                            vertex_vec[i + 2] = (Vertex_0{x, y + 1, current_tile_data.loc.x, current_tile_data.loc.y + current_tile_data.size.y});
                                            vertex_vec[i + 3] = (Vertex_0{x, y + 1, current_tile_data.loc.x, current_tile_data.loc.y + current_tile_data.size.y});
                                            vertex_vec[i + 4] = (Vertex_0{x + 1, y, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y});
                                            vertex_vec[i + 5] = (Vertex_0{x + 1, y + 1, current_tile_data.loc.x + current_tile_data.size.x, current_tile_data.loc.y + current_tile_data.size.y});
                                            i += 6;
                                        }
                                    }
                                }

                                core.paste_buffer.bind();
                                core.paste_buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex_0));
                                core.paste_buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
                                core.paste_buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));
                                
                                core.reload_paste_buffer = false;
                            }
                            core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                        } else {
                            core.start_pos = core.mouse_tile;
                            core.end_pos = core.mouse_tile;
                            
                            core.bottom_left = glm::ivec2(std::min(core.start_pos.x, core.end_pos.x), std::min(core.start_pos.y, core.end_pos.y));
                            core.top_right = glm::ivec2(std::max(core.start_pos.x, core.end_pos.x), std::max(core.start_pos.y, core.end_pos.y));

                            core.select_active = true;
                            core.show_select = true;

                            std::vector<glm::vec2> vertex_vec;

                            vertex_vec.push_back(glm::vec2(core.bottom_left));
                            vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                            vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                            vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                            vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                            vertex_vec.push_back(glm::vec2(core.top_right + 1));

                            core.select_buffer.bind();
                            core.select_buffer.set_data(vertex_vec.data(), 6, sizeof(glm::vec2));
                            core.select_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);

                            core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                        }
                        break;
                    }

                    case PASTE: {
                        if(core.mouse_tile.x >= core.bottom_left.x && core.mouse_tile.x <= core.top_right.x && core.mouse_tile.y >= core.bottom_left.y && core.mouse_tile.y <= core.top_right.y) {
                            core.paste_offset = core.bottom_left - core.mouse_tile;

                            core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                        } else {
                            Action action;
                            std::set<Chunk*> edited_chunks;

                            for(int x = 0; x < core.select_data.size(); x++) {
                                for(int y = 0; y < core.select_data[0].size(); y++) {
                                    int set_tile = core.select_data[x][y];
                                    if(set_tile) {
                                        glm::ivec2 insert_loc(core.bottom_left.x + x, core.bottom_left.y + y);
                                        glm::ivec2 chunk_key = insert_loc >> 4;

                                        if(!core.chunks.contains(chunk_key)) {
                                            core.chunks.insert({chunk_key, Chunk(chunk_key)});
                                        }

                                        glm::ivec2 pos_in_chunk = insert_loc & 15;
                                        Chunk* chunk = &core.chunks[chunk_key];
                                        int& target_tile = chunk->tiles[pos_in_chunk.x + (pos_in_chunk.y << 4)];
                                        
                                        if(target_tile != set_tile) {
                                            action.contents.push_back(Set_action{{chunk_key, pos_in_chunk}, target_tile, set_tile});
                                            target_tile = set_tile;

                                            edited_chunks.insert(chunk);
                                        }
                                    }
                                }
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            core.insert_action(action);

                            core.active_edit_mode = SET;
                            core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;

                            core.paste_offset = {0, 0};
                        }

                        break;
                    }
                }
            } else if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_ON) {
                if(core.active_mode == EDIT && core.active_edit_mode == PASTE) {
                    if(core.select_drag) {

                        std::set<Chunk*> edited_chunks;

                        for(int x = 0; x < core.select_data.size(); x++) {
                            for(int y = 0; y < core.select_data[0].size(); y++) {
                                int set_tile = core.select_data[x][y];
                                if(set_tile) {
                                    glm::ivec2 insert_loc(core.mouse_tile.x + core.paste_offset.x + x, core.mouse_tile.y + core.paste_offset.y + y);
                                    glm::ivec2 chunk_key = insert_loc >> 4;

                                    if(!core.chunks.contains(chunk_key)) {
                                        core.chunks.insert({chunk_key, Chunk(chunk_key)});
                                    }

                                    glm::ivec2 pos_in_chunk = insert_loc & 15;
                                    Chunk* chunk = &core.chunks[chunk_key];
                                    int& target_tile = chunk->tiles[pos_in_chunk.x + (pos_in_chunk.y << 4)];
                                    
                                    if(target_tile != set_tile) {
                                        core.new_action.contents.push_back(Set_action{{chunk_key, pos_in_chunk}, target_tile, set_tile});
                                        target_tile = set_tile;

                                        edited_chunks.insert(chunk);
                                    }
                                }
                            }
                        }

                        for(Chunk* c : edited_chunks) {
                            c->create_vertices(core);
                        }

                        core.insert_action();

                        glm::ivec2 distance_moved = core.mouse_tile + core.paste_offset - core.bottom_left;

                        core.bottom_left += distance_moved;
                        core.top_right += distance_moved;
                        core.select_active = false;

                        // create selection
                        
                        std::vector<glm::vec2> vertex_vec;

                        vertex_vec.push_back(glm::vec2(core.bottom_left));
                        vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                        vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                        vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                        vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                        vertex_vec.push_back(glm::vec2(core.top_right + 1));

                        core.select_buffer.bind();
                        core.select_buffer.set_data(vertex_vec.data(), 6, sizeof(glm::vec2));
                        core.select_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);

                        //

                        core.select_drag = false;
                        core.active_edit_mode = SELECT;
                    } else {

                        glm::ivec2 distance_moved = core.mouse_tile + core.paste_offset - core.bottom_left;

                        core.bottom_left += distance_moved;
                        core.top_right += distance_moved;
                        core.select_active = false;

                        // create selection
                        
                        std::vector<glm::vec2> vertex_vec;

                        vertex_vec.push_back(glm::vec2(core.bottom_left));
                        vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                        vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                        vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                        vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                        vertex_vec.push_back(glm::vec2(core.top_right + 1));

                        core.select_buffer.bind();
                        core.select_buffer.set_data(vertex_vec.data(), 6, sizeof(glm::vec2));
                        core.select_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);

                        //
                    }

                    core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = UP_OFF;
                }
            }

            if(core.keymap_mouse[GLFW_MOUSE_BUTTON_MIDDLE] == PRESS_ON) {
                glm::ivec2 chunk_key = core.mouse_tile >> 4;
                if(core.chunks.contains(chunk_key)) {
                    core.active_tile_id = core.chunks[chunk_key].tiles[(core.mouse_tile.x & 15) + ((core.mouse_tile.y & 15) << 4)];
                } else {
                    core.active_tile_id = 0;
                }
            }
        }

        if(core.select_active) {
            if(core.end_pos != core.mouse_tile) {
                core.end_pos = core.mouse_tile;
                core.bottom_left = glm::ivec2(std::min(core.start_pos.x, core.end_pos.x), std::min(core.start_pos.y, core.end_pos.y));
                core.top_right = glm::ivec2(std::max(core.start_pos.x, core.end_pos.x), std::max(core.start_pos.y, core.end_pos.y));

                std::vector<glm::vec2> vertex_vec;

                vertex_vec.push_back(glm::vec2(core.bottom_left));
                vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                vertex_vec.push_back(glm::vec2(core.top_right.x + 1.0f, core.bottom_left.y));
                vertex_vec.push_back(glm::vec2(core.bottom_left.x, core.top_right.y + 1.0f));
                vertex_vec.push_back(glm::vec2(core.top_right + 1));

                core.select_buffer.bind();
                core.select_buffer.set_data(vertex_vec.data(), 6, sizeof(glm::vec2));
                core.select_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);
            } 
        }

        // -> render

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat3 matrix = glm::translate(identity_matrix, glm::floor(core.camera_pos * float(core.scale)) / float(core.scale));
        matrix = glm::scale(matrix, glm::vec2{core.screen_size} / (float(core.scale) * 2));

        // do not delete!!!
        /*glm::mat3 cam_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};
        cam_matrix = glm::scale(cam_matrix, float(core.scale) / glm::vec2{core.screen_size / 2});
        cam_matrix = glm::translate(cam_matrix, -glm::floor(core.camera_pos * float(core.scale)) / float(core.scale));*/
        glm::mat3 cam_matrix = glm::inverse(matrix);

        chunk_shader.use();
        glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
        core.texture.bind(0, 2);
        for(auto& [key, chunk] : core.chunks) {
            glm::mat3 chunk_matrix = glm::translate(identity_matrix, glm::vec2(chunk.pos * 16));
            glUniformMatrix3fv(0, 1, false, &chunk_matrix[0][0]);
            chunk.buffer.bind();
            glDrawArrays(GL_TRIANGLES, 0, chunk.buffer.vertices);
        }
        
        if(core.active_mode == EDIT) {
            // draw hover

            if(core.active_edit_mode == SET || core.active_edit_mode == FILL) {
                glm::mat3 hover_matrix = glm::translate(identity_matrix, glm::vec2(core.mouse_tile));

                hover_buffer.bind();
                if(core.active_tile_id == 0) {
                    delete_shader.use();
                    glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
                    glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                    glUniform1f(2, cos(double((time_container - start_time) / 1000000) * (0.003 * M_PI)) * 0.125 + 0.375);
                } else {
                    hover_shader.use();
                    glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
                    glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                    glUniform4f(2, core.tile_data[core.active_tile_id].loc.x, core.tile_data[core.active_tile_id].loc.y, core.tile_data[core.active_tile_id].size.x, core.tile_data[core.active_tile_id].size.y);
                    core.texture.bind(0, 3);
                }
                glDrawArrays(GL_TRIANGLES, 0, 6);
            } else if(core.active_edit_mode == SELECT && core.show_select) {
                core.select_buffer.bind();

                select_shader.use();
                glUniformMatrix3fv(0, 1, false, &cam_matrix[0][0]);
                glUniform1f(1, cos(double((time_container - start_time) / 1000000) * (0.003 * M_PI)) * 0.0625 + 0.25);

                glDrawArrays(GL_TRIANGLES, 0, 6);
            } else if(core.active_edit_mode == PASTE) {
                paste_shader.use();
                
                glm::vec2 location;
                if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_OFF) {
                    location = core.mouse_tile + core.paste_offset;
                } else {
                    location = core.bottom_left;
                }

                glm::mat3 paste_matrix = glm::translate(identity_matrix, location);

                core.paste_buffer.bind();

                glUniformMatrix3fv(0, 1, false, &paste_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                core.texture.bind(0, 2);

                glDrawArrays(GL_TRIANGLES, 0, core.paste_buffer.vertices);

                if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_OFF) {
                    core.select_buffer.bind();

                    select_shader.use();
                    glUniformMatrix3fv(0, 1, false, &cam_matrix[0][0]);
                    glUniform1f(1, cos(double((time_container - start_time) / 1000000) * (0.003 * M_PI)) * 0.0625 + 0.25);

                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
            }

            grid_buffer.bind();
            grid_shader.use();
            glUniformMatrix3fv(0, 1, false, &matrix[0][0]);
            glUniform2fv(1, 1, &core.camera_pos[0]);
            glUniform1i(2, core.scale);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glfwSwapBuffers(window);

        core.game_running = !glfwWindowShouldClose(window);
    }

    core.save_game("output\\save.txt");

    glfwTerminate();
}