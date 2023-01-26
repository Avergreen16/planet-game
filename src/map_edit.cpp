#define USE_2D
#include "wrapper.cpp"
#include "fstream"
#include <chrono>
#include <deque>
#include <set>
#include <any>
#include <sstream>

time_t get_time() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

std::string get_text_from_file(char* path) {
    std::ifstream file;
    file.open(path);

    if(file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();

    } else {
        std::cout << "failed to open file " << path << std::endl;
    }
}

struct vec_comp {
    bool operator()(const glm::ivec2 &a, const glm::ivec2 &b) const {
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
    int type;
    glm::ivec2 loc;
    glm::ivec2 size;
};

namespace edit {
    struct Edit_engine {

    };
}

struct Chunk;

enum mode {
    VIEW,
    EDIT
};

enum edit_mode {
    SET,
    FILL,
    SELECT,
    PASTE
};

struct Event {
    std::any contents;
    int id = -1;
    int bundle = 1;
};

// event types

struct Edit_tile {
    glm::ivec2 loc;
    int prev;
    int set;
};

struct Event_edit_tiles {
    std::vector<Edit_tile> tiles_changed;
};

struct Event_select_drag {
    std::vector<Edit_tile> tiles_changed;
    std::set<glm::ivec2, vec_comp> selected_tiles;
    glm::ivec2 selection_size;
    glm::ivec2 start_bottom_left;
    glm::ivec2 end_bottom_left;
};

struct Event_paste {
    std::vector<Edit_tile> tiles_changed;
    std::set<glm::ivec2, vec_comp> selected_tiles;
    glm::ivec2 bottom_left;
    glm::ivec2 top_right;
};

struct Paste_data {
    std::vector<std::vector<int>> data;
    int tile_count = 0;
};

enum key_status {
    UP_ON,
    PRESS_ON,
    UP_OFF,
    PRESS_OFF
};

struct Core {
    GLFWwindow* window;
    std::vector<Buffer> buffers;
    std::vector<Shader> shaders;

    bool game_running = true;
    time_t start_time;
    time_t last_time;
    time_t delta_time;

    glm::ivec2 screen_size = {800, 600};

    glm::ivec2 mouse_tile;

    int active_tile_id = 1;
    int max_tile_id = 0;

    mode active_mode = EDIT;
    edit_mode active_edit_mode = SET;

    bool set_active = false;

    // used for select
    glm::ivec2 start_pos = {0, 0};
    glm::ivec2 end_pos = {0, 0};
    glm::ivec2 bottom_left = {0, 0};
    glm::ivec2 top_right = {0, 0};
    glm::ivec2 prev_bottom_left = {0, 0};

    std::set<glm::ivec2, vec_comp> selected_tiles;
    glm::ivec2 select_bottom_left = {0, 0};
    glm::ivec2 select_top_right = {0, 0};

    Buffer select_buffer;
    Buffer select_mod_buffer;
    bool select_mod_active = false;
    bool show_select = false;

    bool reload_selected_tiles_paste = false;

    std::set<glm::ivec2, vec_comp> selected_tiles_paste;
    glm::ivec2 selected_tiles_paste_bottom_left;

    Paste_data paste_data0;
    Paste_data paste_data1;

    bool select_drag = false;

    bool reload_paste_buffer = true;
    Buffer paste_buffer;
    glm::ivec2 paste_offset = {0, 0};

    // used for undo/redo
    Event_edit_tiles new_edit_event;
    std::deque<Event> event_queue;
    int pos_in_queue = -1;

    std::map<int, key_status> keymap = { 
        {GLFW_KEY_LEFT_CONTROL, UP_OFF},
        {GLFW_KEY_LEFT_SHIFT, UP_OFF},
        {GLFW_KEY_ESCAPE, UP_OFF}
    };

    std::map<int, key_status> keymap_mouse = { 
        {GLFW_MOUSE_BUTTON_RIGHT, UP_OFF}, 
        {GLFW_MOUSE_BUTTON_LEFT, UP_OFF}, 
        {GLFW_MOUSE_BUTTON_MIDDLE, UP_OFF}
    };

    std::vector<Tile_data> tile_data = std::vector<Tile_data>(1);
    // std::vector<Texture> textures;
    Texture texture;

    std::map<glm::ivec2, Chunk, vec_comp> chunks;

    glm::vec2 camera_pos = {0, 0};
    glm::ivec2 mouse_pos = {0, 0};
    int scale = 16;

    void insert_event_edit_tiles() {
        if(new_edit_event.tiles_changed.size() != 0) {
            if(event_queue.size() > pos_in_queue + 1) {
                event_queue.erase(event_queue.begin() + pos_in_queue + 1, event_queue.end());
            }
            pos_in_queue++;
            event_queue.push_back({new_edit_event, 0});

            new_edit_event.tiles_changed.clear();
        }
    }

    void insert_event(Event& e) {
        if(event_queue.size() > pos_in_queue + 1) {
            event_queue.erase(event_queue.begin() + pos_in_queue + 1, event_queue.end());
        }
        pos_in_queue++;
        event_queue.push_back(e);
    }

    void move_selection(glm::ivec2 dist) {
        prev_bottom_left = select_bottom_left;
        select_bottom_left += dist;
        select_top_right += dist;
        
        std::set<glm::ivec2, vec_comp> copied_data = selected_tiles;
        selected_tiles.clear();

        for(glm::ivec2 v : copied_data) {
            selected_tiles.insert(v + dist);
        }
    }

    void update_select_buffer() {
        std::vector<glm::vec2> vertex_vec;

        for(glm::ivec2 pos : selected_tiles) {
            glm::ivec2 new_pos = pos - select_bottom_left;
            vertex_vec.push_back(glm::vec2(new_pos));
            vertex_vec.push_back(glm::vec2(new_pos + glm::ivec2{1, 0}));
            vertex_vec.push_back(glm::vec2(new_pos + glm::ivec2{0, 1}));
            vertex_vec.push_back(glm::vec2(new_pos + glm::ivec2{0, 1}));
            vertex_vec.push_back(glm::vec2(new_pos + glm::ivec2{1, 0}));
            vertex_vec.push_back(glm::vec2(new_pos + glm::ivec2{1, 1}));
        }

        select_buffer.bind();
        select_buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(glm::vec2));
        select_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);
    }

    void update_select_mod_buffer() {   
        std::vector<glm::vec2> vertex_vec;

        vertex_vec.push_back(glm::vec2(bottom_left));
        vertex_vec.push_back(glm::vec2(top_right.x + 1, bottom_left.y));
        vertex_vec.push_back(glm::vec2(bottom_left.x, top_right.y + 1));
        vertex_vec.push_back(glm::vec2(bottom_left.x, top_right.y + 1));
        vertex_vec.push_back(glm::vec2(top_right.x + 1, bottom_left.y));
        vertex_vec.push_back(glm::vec2(top_right + 1));

        select_mod_buffer.bind();
        select_mod_buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(glm::vec2));
        select_mod_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);
    }

    void update_select_boundaries(glm::ivec2 new_bottom_left, glm::ivec2 new_top_right) {
        prev_bottom_left = select_bottom_left;
        select_bottom_left = new_bottom_left;
        select_top_right = new_top_right;
    }

    void insert_event_select_drag() {
        if(new_edit_event.tiles_changed.size() != 0) {
            if(event_queue.size() > pos_in_queue + 1) {
                event_queue.erase(event_queue.begin() + pos_in_queue + 1, event_queue.end());
            }

            event_queue.push_back(Event());
            Event& e0 = event_queue[event_queue.size() - 1];
            e0.id = 1;
            e0.contents = Event_select_drag();
                
            Event_select_drag& e1 = *std::any_cast<Event_select_drag>(&e0.contents);

            e1.tiles_changed = new_edit_event.tiles_changed;
            e1.start_bottom_left = prev_bottom_left;
            e1.end_bottom_left = select_bottom_left;
            e1.selection_size = select_top_right - select_bottom_left + glm::ivec2{1, 1};
            e1.selected_tiles = selected_tiles;

            new_edit_event.tiles_changed.clear();

            pos_in_queue++;
        }
    }

    void insert_event_paste() {
        if(new_edit_event.tiles_changed.size() != 0) {
            if(event_queue.size() > pos_in_queue + 1) {
                event_queue.erase(event_queue.begin() + pos_in_queue + 1, event_queue.end());
            }

            event_queue.push_back(Event());
            Event& e0 = event_queue[event_queue.size() - 1];
            e0.id = 2;
            e0.contents = Event_paste();
                
            Event_paste& e1 = *std::any_cast<Event_paste>(&e0.contents);

            e1.tiles_changed = new_edit_event.tiles_changed;
            e1.bottom_left = select_bottom_left;
            e1.top_right = select_top_right;
            e1.selected_tiles = selected_tiles;

            new_edit_event.tiles_changed.clear();

            pos_in_queue++;
        }
    }

    void add_selected_tiles(bool reset_boundaries) {
        if(reset_boundaries || selected_tiles.size() == 0) {
            update_select_boundaries(bottom_left, top_right);
        } else {
            update_select_boundaries(glm::ivec2(std::min(select_bottom_left.x, bottom_left.x), std::min(select_bottom_left.y, bottom_left.y)), glm::ivec2(std::max(select_top_right.x, top_right.x), std::max(select_top_right.y, top_right.y)));
        }

        bool update_buffer = false;

        for(int x = bottom_left.x; x <= top_right.x; x++) {
            for(int y = bottom_left.y; y <= top_right.y; y++) {
                if(selected_tiles.insert({x, y}).second) {
                    update_buffer = true;
                }
            }
        }

        if(update_buffer) {
            update_select_buffer();
        }
        
        show_select = true;
    }

    void subtract_selected_tiles() {
        if(selected_tiles.size() != 0) {
            bool update_buffer = false;

            for(int x = bottom_left.x; x <= top_right.x; x++) {
                for(int y = bottom_left.y; y <= top_right.y; y++) {
                    if(selected_tiles.erase({x, y})) {
                        update_buffer = true;
                    }
                }
            }

            if(selected_tiles.size() != 0) {
                select_bottom_left.y = (*selected_tiles.begin()).y;
                select_top_right.y = (*prev(selected_tiles.end(), 1)).y;

                for(glm::ivec2 v : selected_tiles) {
                    select_bottom_left.x = std::min(select_bottom_left.x, v.x);
                    select_top_right.x = std::max(select_top_right.x, v.x);
                }
            }

            if(update_buffer) {
                update_select_buffer();
            }
        }
    }

    void update_paste_buffer(Paste_data& paste_data) {
        std::vector<Vertex_0> vertex_vec(paste_data.tile_count * 6);
        int i = 0;

        for(int x = 0; x < paste_data.data.size(); x++) {
            for(int y = 0; y < paste_data.data[0].size(); y++) {
                int tile = paste_data.data[x][y];
                if(tile) {

                    Tile_data &current_tile_data = tile_data[tile];

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

        paste_buffer.bind();
        paste_buffer.set_data(vertex_vec.data(), vertex_vec.size(), sizeof(Vertex_0));
        paste_buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
        paste_buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));
    }

    void load_paste_data(Paste_data& data);

    void insert_paste_data(glm::ivec2 paste_loc, Paste_data& data);

    void delete_selection(bool update_action);

    void load_data();

    void save_game(std::string address);

    void game_loop();
};

struct Chunk {
    std::array<uint16_t, 512> tiles;
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

        for(int i = 0; i < 512; i++) {
            int tile = tiles[i];

            if(tile) {
                Tile_data &current_tile_data = core.tile_data[tile];
                int x = i % 16;
                int y = (i / 16) % 16;

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
        while (!end) {
            data >> string;
            if(strcmp(string, "end") == 0) {
                end = true;
            } else if(strcmp(string, "tile") == 0) {
                int tile_pos = tile_data.size();
                tile_data.push_back(Tile_data());
                data >> tile_data[tile_pos].type >> tile_data[tile_pos].loc.x >> tile_data[tile_pos].loc.y >> tile_data[tile_pos].size.x >> tile_data[tile_pos].size.y;

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
    save.open("saves\\world.bin", std::ios::in | std::ios::binary);

    if(save.is_open()) {
        uint16_t info_id;
        bool end = false;
        while (!end) {
            save.read((char*)&info_id, 2);

            if(info_id == (uint16_t)0xFFFF) {
                end = true;
            } else if(info_id == (uint16_t)0x0000) {
                glm::ivec2 loc;
                save.read((char*)&loc, 8);

                chunks.insert({loc, Chunk(loc)});

                Chunk& chunk = chunks[loc];

                int i = 0;

                while(i < 512) {
                    uint16_t id;
                    uint16_t count;

                    save.read((char*)&id, 2);
                    save.read((char*)&count, 2);

                    for(int j = 0; j < count; j++) {
                        chunk.tiles[i + j] = id;
                    }

                    i += count;
                }
            } else {
                end = true;
                std::cout << "input error" << std::endl;
            }
        }
        save.close();
    } else {
        std::cout << "unable to open save" << std::endl;
    }
}

void Core::save_game(std::string address) {
    std::ofstream output;
    output.open(address, std::ios::out | std::ios::trunc | std::ios::binary);

    std::vector<uint16_t> byte_vec;

    if(output.is_open()) {
        for(auto& [key, chunk] : chunks) {
            std::vector<uint16_t> chunk_byte_vec;

            chunk_byte_vec.push_back((uint16_t)0);
            chunk_byte_vec.push_back(chunk.pos.x & 0xFFFF);
            chunk_byte_vec.push_back(chunk.pos.x >> 16);
            chunk_byte_vec.push_back(chunk.pos.y & 0xFFFF);
            chunk_byte_vec.push_back(chunk.pos.y >> 16);

            uint16_t id = chunk.tiles[0];
            uint16_t count = 0;
            for(int i = 0; i < 512; i++) {
                int current_id = chunk.tiles[i];

                if(id != current_id) {
                    chunk_byte_vec.push_back(id);
                    chunk_byte_vec.push_back(count);
                    id = current_id;
                    count = 0;
                }

                count++;
            }
            if(!(id == 0 && count == 512)) {
                for(uint16_t v : chunk_byte_vec) {
                    byte_vec.push_back(v);
                }

                byte_vec.push_back(id);
                byte_vec.push_back(count);
            }
        }
    } else {
        std::cout << "output error" << std::endl;
    }

    byte_vec.push_back((uint16_t)0xFFFF);

    output.write((const char*)byte_vec.data(), byte_vec.size() * 2);

    output.close();
}

void Core::load_paste_data(Paste_data& paste_data) {
    paste_data.data.clear();
    paste_data.data.resize(select_top_right.x - select_bottom_left.x + 1);
    paste_data.tile_count = 0;

    int y_dif = select_top_right.y - select_bottom_left.y + 1;

    for(int i = 0; i < paste_data.data.size(); i++) {
        paste_data.data[i] = std::vector<int>(y_dif);
    }

    int count = 0;

    for(glm::ivec2 v : selected_tiles) {
        int rel_x = v.x - select_bottom_left.x;
        glm::ivec2 chunk_key(v.x >> 4, v.y >> 4);
        if(chunks.contains(chunk_key)) {
            int tile = chunks[chunk_key].tiles[(v.x & 15) + ((v.y & 15) << 4)];
            if(tile)
                paste_data.tile_count++;
            paste_data.data[rel_x][v.y - select_bottom_left.y] = tile;
        }
        count++;
    }

    reload_paste_buffer = true;
}

void Core::insert_paste_data(glm::ivec2 paste_loc, Paste_data& paste_data) {
    std::set<Chunk*> edited_chunks;

    for(int x = 0; x < paste_data.data.size(); x++) {
        for(int y = 0; y < paste_data.data[0].size(); y++) {
            int set_tile = paste_data.data[x][y];
            if(set_tile) {
                glm::ivec2 insert_loc(paste_loc.x + x, paste_loc.y + y);
                glm::ivec2 chunk_key = insert_loc >> 4;

                if(!chunks.contains(chunk_key)) {
                    chunks.insert({chunk_key, Chunk(chunk_key)});
                }

                glm::ivec2 pos_in_chunk = insert_loc & 15;
                Chunk* chunk = &chunks[chunk_key];
                uint16_t &target_tile = chunk->tiles[pos_in_chunk.x + (pos_in_chunk.y << 4)];

                if(target_tile != set_tile) {
                    new_edit_event.tiles_changed.push_back(Edit_tile{insert_loc, target_tile, set_tile});
                    target_tile = set_tile;

                    edited_chunks.insert(chunk);
                }
            }
        }
    }

    for(Chunk* c : edited_chunks) {
        c->create_vertices(*this);
    }
}

void Core::delete_selection(bool update_action) {
    std::set<Chunk*> edited_chunks;

    for(glm::ivec2 v : selected_tiles) {
        glm::ivec2 chunk_key(v.x >> 4, v.y >> 4);

        if(chunks.contains(chunk_key)) {
            Chunk& chunk = chunks[chunk_key];
            int pos_in_chunk = (v.x & 15) + ((v.y & 15) << 4);
            uint16_t& tile = chunk.tiles[pos_in_chunk];
            if(tile) {
                new_edit_event.tiles_changed.push_back(Edit_tile{v, tile, 0});
                tile = 0;
            }

            edited_chunks.insert(&chunk);
        }
    }

    for(Chunk* c : edited_chunks) {
        c->create_vertices(*this);
    }

    if(update_action) {
        // backspace event
        if(new_edit_event.tiles_changed.size() != 0) {
            if(event_queue.size() > pos_in_queue + 1) {
                event_queue.erase(event_queue.begin() + pos_in_queue + 1, event_queue.end());
            }

            event_queue.push_back(Event());
            Event& e0 = event_queue[event_queue.size() - 1];
            e0.id = 3;
            e0.contents = Event_paste();
                
            Event_paste& e1 = *std::any_cast<Event_paste>(&e0.contents);

            e1.tiles_changed = new_edit_event.tiles_changed;
            e1.bottom_left = select_bottom_left;
            e1.top_right = select_top_right;
            e1.selected_tiles = selected_tiles;

            new_edit_event.tiles_changed.clear();

            pos_in_queue++;
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Core& core = *(Core*)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS) {

        if(core.keymap.contains(key)) {
            core.keymap[key] = PRESS_ON;
        } else if(key > 47 && key <= 48 + core.max_tile_id) {
            core.active_tile_id = key - 48;
        } else if(key == GLFW_KEY_COMMA) {
            if(core.active_tile_id > 0) core.active_tile_id--;
        } else if(key == GLFW_KEY_PERIOD) {
            if(core.active_tile_id < core.max_tile_id) core.active_tile_id++;
        } else if(key == GLFW_KEY_F11) {
            std::vector<uint8_t> vector(core.screen_size.x * core.screen_size.y * 3);
            glReadPixels(0, 0, core.screen_size.x, core.screen_size.y, GL_RGB, GL_UNSIGNED_BYTE, vector.data());

            stbi_write_png(("screenshots\\screenshot" + std::to_string(int(get_time() / 100000000)) + ".png").data(), core.screen_size.x, core.screen_size.y, 3, vector.data(), core.screen_size.x * 3);
        } else if(key == GLFW_KEY_P) {
            if(core.active_mode == VIEW) {
                core.active_mode = EDIT;
            } else {
                core.active_mode = VIEW;
                if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] & 1 && core.new_edit_event.tiles_changed.size() != 0) {
                    core.insert_event_edit_tiles();
                }
            }
        } else if(key == GLFW_KEY_F) {
            switch(core.active_edit_mode) {
                case SET:
                    core.active_edit_mode = FILL;
                    core.insert_event_edit_tiles();
                    core.set_active = false;
                    break;
                case FILL:
                    core.active_edit_mode = SELECT;
                    break;
                case SELECT:
                    core.select_mod_active = false;
                    core.show_select = false;
                    core.selected_tiles.clear();
                    core.active_edit_mode = SET;
                    break;
                case PASTE:
                    core.active_edit_mode = SET;
                    break;
            }
        } else if(key == GLFW_KEY_BACKSPACE) {
            if(core.active_mode == EDIT && core.active_edit_mode == SELECT && core.show_select) {
                core.delete_selection(true);
                core.show_select = false;
            }
        }

        if(core.keymap[GLFW_KEY_LEFT_CONTROL] & 1) {
            if(key == GLFW_KEY_Z) { // undo
                if(core.pos_in_queue != -1) {
                    Event& event = core.event_queue[core.pos_in_queue];
                    switch(event.id) {
                        case 0: {
                            Event_edit_tiles& e = *std::any_cast<Event_edit_tiles>(&event.contents);

                            std::set<Chunk*> edited_chunks;

                            for(auto t = --e.tiles_changed.end(); t != --e.tiles_changed.begin(); --t) {
                                Chunk& chunk = core.chunks[t->loc >> 4];
                                chunk.tiles[(t->loc.x & 15) + ((t->loc.y & 15) << 4)] = t->prev;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            core.show_select = false;
                            core.selected_tiles.clear();

                            break;
                        }
                        case 1: {
                            Event_select_drag& e = *std::any_cast<Event_select_drag>(&event.contents);

                            core.selected_tiles = e.selected_tiles;

                            core.move_selection(e.start_bottom_left - e.end_bottom_left);
                            core.update_select_buffer();

                            core.active_edit_mode = SELECT;
                            core.show_select = true;

                            //

                            std::set<Chunk*> edited_chunks;

                            for(auto t = --e.tiles_changed.end(); t != --e.tiles_changed.begin(); --t) {
                                Chunk& chunk = core.chunks[t->loc >> 4];
                                chunk.tiles[(t->loc.x & 15) + ((t->loc.y & 15) << 4)] = t->prev;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            break;
                        }
                        case 2: {
                            Event_paste& e = *std::any_cast<Event_paste>(&event.contents);

                            //

                            std::set<Chunk*> edited_chunks;

                            for(auto t = --e.tiles_changed.end(); t != --e.tiles_changed.begin(); --t) {
                                Chunk& chunk = core.chunks[t->loc >> 4];
                                chunk.tiles[(t->loc.x & 15) + ((t->loc.y & 15) << 4)] = t->prev;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            core.show_select = false;
                            core.selected_tiles.clear();

                            break;
                        }
                        case 3: {
                            Event_paste& e = *std::any_cast<Event_paste>(&event.contents);

                            core.select_bottom_left = e.bottom_left;
                            core.select_top_right = e.top_right;
                            core.selected_tiles = e.selected_tiles;
                            core.active_edit_mode = SELECT;
                            core.show_select = true;

                            core.update_select_buffer();

                            //

                            std::set<Chunk*> edited_chunks;

                            for(auto t = --e.tiles_changed.end(); t != --e.tiles_changed.begin(); --t) {
                                Chunk& chunk = core.chunks[t->loc >> 4];
                                chunk.tiles[(t->loc.x & 15) + ((t->loc.y & 15) << 4)] = t->prev;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            break;
                        }
                    }

                    core.pos_in_queue--;
                }
            } else if(key == GLFW_KEY_Y) { // redo
                if(core.pos_in_queue != core.event_queue.size() - 1) {
                    core.pos_in_queue++;

                    Event& event = core.event_queue[core.pos_in_queue];
                    switch(event.id) {
                        case 0: {
                            Event_edit_tiles& e = *std::any_cast<Event_edit_tiles>(&event.contents);

                            std::set<Chunk*> edited_chunks;

                            for(Edit_tile& t : e.tiles_changed) {
                                Chunk& chunk = core.chunks[t.loc >> 4];
                                chunk.tiles[(t.loc.x & 15) + ((t.loc.y & 15) << 4)] = t.set;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }
                            break;
                        }
                        case 1: {
                            Event_select_drag& e = *std::any_cast<Event_select_drag>(&event.contents);

                            core.select_bottom_left = e.end_bottom_left;
                            core.select_top_right = e.end_bottom_left + e.selection_size + glm::ivec2{-1, -1};
                            core.selected_tiles = e.selected_tiles;
                            core.active_edit_mode = SELECT;
                            core.show_select = true;

                            core.update_select_buffer();

                            //

                            std::set<Chunk*> edited_chunks;

                            for(Edit_tile& t : e.tiles_changed) {
                                Chunk& chunk = core.chunks[t.loc >> 4];
                                chunk.tiles[(t.loc.x & 15) + ((t.loc.y & 15) << 4)] = t.set;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            break;
                        }
                        case 2: {
                            Event_paste& e = *std::any_cast<Event_paste>(&event.contents);

                            core.select_bottom_left = e.bottom_left;
                            core.select_top_right = e.top_right;
                            core.selected_tiles = e.selected_tiles;
                            core.active_edit_mode = SELECT;
                            core.show_select = true;

                            core.update_select_buffer();

                            //

                            std::set<Chunk*> edited_chunks;

                            for(Edit_tile& t : e.tiles_changed) {
                                Chunk& chunk = core.chunks[t.loc >> 4];
                                chunk.tiles[(t.loc.x & 15) + ((t.loc.y & 15) << 4)] = t.set;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            break;
                        }
                        case 3: {
                            Event_paste& e = *std::any_cast<Event_paste>(&event.contents);

                            //

                            std::set<Chunk*> edited_chunks;

                            for(Edit_tile& t : e.tiles_changed) {
                                Chunk& chunk = core.chunks[t.loc >> 4];
                                chunk.tiles[(t.loc.x & 15) + ((t.loc.y & 15) << 4)] = t.set;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }

                            core.show_select = false;
                            core.selected_tiles.clear();

                            break;
                        }
                    }
                }
            } else if(key == GLFW_KEY_C) { // copy
                if(core.show_select == true && core.active_edit_mode == SELECT) {
                    core.load_paste_data(core.paste_data1);
                    core.selected_tiles_paste = core.selected_tiles;
                    core.selected_tiles_paste_bottom_left = core.select_bottom_left;

                    core.reload_paste_buffer = true;
                }
            } else if(key == GLFW_KEY_V) { // paste
                if(core.active_mode == EDIT && core.paste_data1.data.size() != 0) {
                    switch(core.active_edit_mode) {
                        case SET: {
                            core.insert_event_edit_tiles();
                            core.set_active = false;
                            break;
                        }
                        case SELECT: {
                            core.select_mod_active = false;
                            break;
                        }
                        case PASTE: {
                            if(core.select_drag) {
                                core.insert_paste_data(core.paste_offset + core.mouse_tile, core.paste_data0);
                                core.insert_event_select_drag();
                            } else {
                                if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_OFF) {
                                    core.insert_paste_data(core.select_bottom_left, core.paste_data1);
                                    core.insert_event_paste();
                                } else {
                                    core.insert_paste_data(core.paste_offset + core.mouse_tile, core.paste_data1);
                                    core.insert_event_paste();
                                }
                            }
                            
                            break;
                        }
                    }

                    core.paste_offset = -glm::ivec2{core.paste_data1.data.size(), core.paste_data1.data[0].size()} / 2;

                    core.selected_tiles = core.selected_tiles_paste;
                    core.select_bottom_left = core.selected_tiles_paste_bottom_left;
                    core.move_selection(core.mouse_tile + core.paste_offset - core.select_bottom_left);
                    core.update_select_buffer();

                    core.active_edit_mode = PASTE;

                    if(core.reload_paste_buffer == true) {
                        core.update_paste_buffer(core.paste_data1);
                        core.reload_paste_buffer = false;
                    }
                }
            } else if(key == GLFW_KEY_S) {
                core.save_game("saves\\world.bin");
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
    if(core.keymap_mouse[GLFW_MOUSE_BUTTON_RIGHT] & 1) {
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

    core.game_loop();

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
                core.insert_event_edit_tiles();
                core.set_active = false;
            } else if(core.active_edit_mode == SELECT) {
                if(core.keymap[GLFW_KEY_LEFT_SHIFT] & 1) {
                    core.add_selected_tiles(false);
                } else if(core.keymap[GLFW_KEY_LEFT_CONTROL] & 1) {
                    core.subtract_selected_tiles();
                } else {
                    core.selected_tiles.clear();
                    if(core.start_pos != core.end_pos) {
                        core.add_selected_tiles(true);
                    } else {
                        core.show_select = false;
                    }
                } // flag :)

                core.select_mod_active = false;
            }
        }
    }
}

glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

void Core::game_loop() {
    // get mouse position in world
    glm::vec2 mouse_pos_world = glm::vec2(mouse_pos.x - screen_size.x / 2, -mouse_pos.y - 1 + screen_size.y / 2);
    mouse_pos_world = mouse_pos_world / float(scale) + camera_pos;
    mouse_tile = glm::floor(mouse_pos_world);

    if(active_mode == EDIT) {
        switch(active_edit_mode) {
            case SET: {
                if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON || (set_active && keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_OFF)) {
                    set_active = true;

                    glm::ivec2 chunk_key = mouse_tile >> 4;

                    if(!chunks.contains(chunk_key)) {
                        chunks.insert({chunk_key, Chunk(chunk_key)});
                    }

                    Tile_data& t_data = tile_data[active_tile_id];
                    int pos_in_chunk = (mouse_tile.x & 15) + ((mouse_tile.y & 15) << 4);
                    Chunk* chunk = &chunks[chunk_key];

                    if(active_tile_id == 0 && keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON) {
                        if(chunk->tiles[pos_in_chunk + 256]) {
                            tile_data[0].type = 1;
                        } else {
                            tile_data[0].type = 0;
                        }
                    }
                    uint16_t& focus_tile = chunk->tiles[pos_in_chunk + 256 * t_data.type];

                    if(focus_tile != active_tile_id) {
                        new_edit_event.tiles_changed.push_back(Edit_tile(mouse_tile, focus_tile, active_tile_id));

                        focus_tile = active_tile_id;
                        chunk->create_vertices(*this);
                    }
                }
                break;
            }

            case FILL: {
                if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON) {
                    mouse_tile;

                    glm::ivec2 chunk_key = mouse_tile >> 4;

                    if(!chunks.contains(chunk_key)) {
                        chunks.insert({chunk_key, Chunk(chunk_key)});
                    }

                    int focus_id = chunks[chunk_key].tiles[(mouse_tile.x & 15) + ((mouse_tile.y & 15) << 4)];

                    if(focus_id != active_tile_id) {
                        std::set<Chunk*> edited_chunks;

                        std::queue<glm::ivec2> queue;
                        queue.push(mouse_tile);
                        int counter = 0;
                        while (queue.size() != 0 && counter < 2113) {
                            glm::ivec2 current_loc = queue.front();
                            queue.pop();

                            glm::ivec2 chunk_key = current_loc >> 4;
                            if(!chunks.contains(chunk_key)) {
                                chunks.insert({chunk_key, Chunk(chunk_key)});
                            }
                            Chunk* chunk = &chunks[chunk_key];

                            uint16_t& tile = chunk->tiles[(current_loc.x & 15) + ((current_loc.y & 15) << 4)];

                            if(tile == focus_id) {
                                counter++;

                                tile = active_tile_id;
                                new_edit_event.tiles_changed.push_back(Edit_tile(current_loc, focus_id, active_tile_id));

                                glm::ivec2 north = current_loc;
                                glm::ivec2 east = current_loc;
                                glm::ivec2 south = current_loc;
                                glm::ivec2 west = current_loc;

                                north.y++;
                                east.x++;
                                south.y--;
                                west.x--;

                                queue.push(north);
                                queue.push(east);
                                queue.push(south);
                                queue.push(west);
                            }

                            edited_chunks.insert(chunk);
                        }

                        for(Chunk* c : edited_chunks) {
                            c->create_vertices(*this);
                        }

                        insert_event_edit_tiles();
                    }
                }
                break;
            }

            case SELECT: {
                if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON) {
                    if(show_select && !select_mod_active && keymap[GLFW_KEY_LEFT_SHIFT] != PRESS_ON && keymap[GLFW_KEY_LEFT_CONTROL] != PRESS_ON && selected_tiles.contains(mouse_tile)) {
                        select_drag = true;
                        paste_offset = select_bottom_left - mouse_tile;

                        //

                        load_paste_data(paste_data0);

                        delete_selection(false);

                        //

                        active_edit_mode = PASTE;

                        update_paste_buffer(paste_data0);
                        reload_paste_buffer = true;
                    } else {
                        start_pos = mouse_tile;
                        end_pos = mouse_tile;

                        bottom_left = glm::ivec2(std::min(start_pos.x, end_pos.x), std::min(start_pos.y, end_pos.y));
                        top_right = glm::ivec2(std::max(start_pos.x, end_pos.x), std::max(start_pos.y, end_pos.y));

                        select_mod_active = true;

                        update_select_mod_buffer();
                    }
                }
                break;
            }

            case PASTE: {
                if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON) {
                    if(selected_tiles.contains(mouse_tile)) {
                        paste_offset = select_bottom_left - mouse_tile;

                    } else {
                        insert_paste_data(select_bottom_left, paste_data1);

                        insert_event_paste();

                        active_edit_mode = SET;

                        paste_offset = {0, 0};
                    }
                } else if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_ON) {
                    if(select_drag) {
                        glm::ivec2 distance_moved = mouse_tile + paste_offset - select_bottom_left;

                        move_selection(distance_moved);

                        insert_paste_data(select_bottom_left, paste_data0);

                        insert_event_select_drag();

                        //

                        select_drag = false;
                        active_edit_mode = SELECT;
                    } else {
                        glm::ivec2 distance_moved = mouse_tile + paste_offset - select_bottom_left;

                        move_selection(distance_moved);
                    }
                }

                break;
            }
        }

        if(keymap_mouse[GLFW_MOUSE_BUTTON_MIDDLE] == PRESS_ON) {
            glm::ivec2 chunk_key = mouse_tile >> 4;
            if(chunks.contains(chunk_key)) {
                int loc = (mouse_tile.x & 15) + ((mouse_tile.y & 15) << 4);

                int id = chunks[chunk_key].tiles[loc + 256];
                if(id) {
                    active_tile_id = id;
                } else {
                    active_tile_id = chunks[chunk_key].tiles[loc];
                }
            } else {
                active_tile_id = 0;
            }
        }
    }

    if(keymap[GLFW_KEY_ESCAPE] == PRESS_ON) {
        if(active_mode == EDIT) {
            switch(active_edit_mode) {
                case SET: {
                    if(set_active) {
                        insert_event_edit_tiles();
                        set_active = false;
                    }                            
                    break;
                }

                case SELECT: {
                    show_select = false;
                    selected_tiles.clear();
                    break;
                }

                case PASTE: {
                    if(select_drag) {
                        glm::ivec2 distance_moved = mouse_tile + paste_offset - select_bottom_left;

                        move_selection(distance_moved);

                        insert_paste_data(select_bottom_left, paste_data0);

                        insert_event_select_drag();

                        //

                        select_drag = false;
                    } else {
                        if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_OFF) {
                            insert_paste_data(select_bottom_left, paste_data1);
                            insert_event_paste();
                            paste_offset = {0, 0};
                        } else {
                            glm::ivec2 distance_moved = mouse_tile + paste_offset - select_bottom_left;

                            move_selection(distance_moved);

                            insert_paste_data(select_bottom_left, paste_data1);

                            insert_event_paste();
                        }
                    }
                }
            }

            active_edit_mode = SET;
        }
    }

    if(select_mod_active) {
        if(end_pos != mouse_tile) {
            end_pos = mouse_tile;
            bottom_left = glm::ivec2(std::min(start_pos.x, end_pos.x), std::min(start_pos.y, end_pos.y));
            top_right = glm::ivec2(std::max(start_pos.x, end_pos.x), std::max(start_pos.y, end_pos.y));

            update_select_mod_buffer();
        }
    }
    
    // keymap changes

    for(auto& pair : keymap_mouse) {
        if(pair.second == PRESS_ON) pair.second = PRESS_OFF;
        else if(pair.second == UP_ON) pair.second = UP_OFF;
    }
    for(auto& pair : keymap) {
        if(pair.second == PRESS_ON) pair.second = PRESS_OFF;
        else if(pair.second == UP_ON) pair.second = UP_OFF;
    }

    // -> render

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat3 matrix = glm::translate(identity_matrix, glm::floor(camera_pos * float(scale)) / float(scale));
    matrix = glm::scale(matrix, glm::vec2{screen_size} / (float(scale) * 2));

    // do not delete!!!
    /*glm::mat3 cam_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    cam_matrix = glm::scale(cam_matrix, float(scale) / glm::vec2{screen_size / 2});
    cam_matrix = glm::translate(cam_matrix, -glm::floor(camera_pos * float(scale)) / float(scale));*/
    glm::mat3 cam_matrix = glm::inverse(matrix);

    shaders[2].use();
    glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
    for(auto& [key, chunk] : chunks) {
        glm::mat3 chunk_matrix = glm::translate(identity_matrix, glm::vec2(chunk.pos << 4));
        glUniformMatrix3fv(0, 1, false, &chunk_matrix[0][0]);
        chunk.buffer.bind();
        glDrawArrays(GL_TRIANGLES, 0, chunk.buffer.vertices);
    }

    if(active_mode == EDIT) {
        // draw hover

        if(active_edit_mode == SET || active_edit_mode == FILL) {
            glm::mat3 hover_matrix = glm::translate(identity_matrix, glm::vec2(mouse_tile));

            buffers[1].bind();
            if(active_tile_id == 0) {
                shaders[3].use();
                glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                glUniform1f(2, cos(double((last_time - start_time) / 1000000) * (0.003 * M_PI)) * 0.125 + 0.375);
            } else {
                shaders[1].use();
                glUniformMatrix3fv(0, 1, false, &hover_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                glUniform4f(2, tile_data[active_tile_id].loc.x, tile_data[active_tile_id].loc.y, tile_data[active_tile_id].size.x, tile_data[active_tile_id].size.y);
            }
            glDrawArrays(GL_TRIANGLES, 0, 6);
        } else if(active_edit_mode == SELECT) {
            if(select_mod_active) {
                select_mod_buffer.bind();

                shaders[4].use();

                glUniformMatrix3fv(0, 1, false, &identity_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                glUniform4f(2, 0.25f, 1.0f, 1.0f, cos(double((last_time - start_time) / 1000000) * (0.003 * M_PI)) * 0.0625 + 0.25);

                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            if(show_select) {
                glm::mat3 pos_matrix = glm::translate(identity_matrix, glm::vec2(select_bottom_left));

                select_buffer.bind();

                shaders[4].use();
                glUniformMatrix3fv(0, 1, false, &pos_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                glUniform4f(2, 0.25f, 1.0f, 1.0f, cos(double((last_time - start_time) / 1000000) * (0.003 * M_PI)) * 0.0625 + 0.25);

                glDrawArrays(GL_TRIANGLES, 0, select_buffer.vertices);
            }
        } else if(active_edit_mode == PASTE) {
            shaders[5].use();

            glm::vec2 location;
            if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_OFF) {
                location = mouse_tile + paste_offset;
            } else {
                location = select_bottom_left;
            }

            glm::mat3 paste_matrix = glm::translate(identity_matrix, location);

            paste_buffer.bind();

            glUniformMatrix3fv(0, 1, false, &paste_matrix[0][0]);
            glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, paste_buffer.vertices);

            if(keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_OFF) {
                select_buffer.bind();

                shaders[4].use();
                glUniformMatrix3fv(0, 1, false, &paste_matrix[0][0]);
                glUniformMatrix3fv(1, 1, false, &cam_matrix[0][0]);
                glUniform4f(2, 0.25f, 1.0f, 1.0f, cos(double((last_time - start_time) / 1000000) * (0.003 * M_PI)) * 0.0625 + 0.25);

                glDrawArrays(GL_TRIANGLES, 0, select_buffer.vertices);
            }
        }

        buffers[0].bind();
        shaders[0].use();
        glUniformMatrix3fv(0, 1, false, &matrix[0][0]);
        glUniform2fv(1, 1, &camera_pos[0]);
        glUniform1i(2, scale);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glfwSwapBuffers(window);
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

    // load data
    core.load_data();

    glfwSetWindowUserPointer(core.window, &core);

    core.buffers = std::vector<Buffer>(2);
    core.shaders = std::vector<Shader>(6);

    Buffer& grid_buffer = core.buffers[0];
    grid_buffer.init();
    grid_buffer.set_data(screen_vertices, 6, sizeof(glm::vec2));
    grid_buffer.set_attrib(0, 2, 2 * sizeof(float), 0);

    Buffer& hover_buffer = core.buffers[1];
    hover_buffer.init();
    hover_buffer.set_data(tile_vertices, 6, sizeof(Vertex_0));
    hover_buffer.set_attrib(0, 2, 4 * sizeof(float), 0);
    hover_buffer.set_attrib(1, 2, 4 * sizeof(float), 2 * sizeof(float));

    core.select_buffer.init();
    core.select_mod_buffer.init();
    core.paste_buffer.init();

    Shader& grid_shader = core.shaders[0];
    grid_shader.compile(get_text_from_file("res\\shaders\\grid.vs").data(), get_text_from_file("res\\shaders\\grid.fs").data());

    Shader& hover_shader = core.shaders[1];
    hover_shader.compile(get_text_from_file("res\\shaders\\hover.vs").data(), get_text_from_file("res\\shaders\\hover.fs").data());

    Shader& chunk_shader = core.shaders[2];
    chunk_shader.compile(get_text_from_file("res\\shaders\\chunk.vs").data(), get_text_from_file("res\\shaders\\chunk.fs").data());

    Shader& delete_shader = core.shaders[3];
    delete_shader.compile(get_text_from_file("res\\shaders\\delete.vs").data(), get_text_from_file("res\\shaders\\delete.fs").data());

    Shader& select_shader = core.shaders[4];
    select_shader.compile(get_text_from_file("res\\shaders\\select.vs").data(), get_text_from_file("res\\shaders\\select.fs").data());

    Shader& paste_shader = core.shaders[5];
    paste_shader.compile(get_text_from_file("res\\shaders\\paste.vs").data(), get_text_from_file("res\\shaders\\paste.fs").data());

    // set up stbi and load textures
    stbi_set_flip_vertically_on_load(true);
    stbi__flip_vertically_on_write = true;

    core.texture.load("res\\tile.png");

    // set callbacks
    glfwSetMouseButtonCallback(core.window, mouse_button_callback);
    glfwSetKeyCallback(core.window, key_callback);
    glfwSetCursorPosCallback(core.window, cursor_pos_callback);
    glfwSetScrollCallback(core.window, scroll_callback);
    glfwSetFramebufferSizeCallback(core.window, framebuffer_size_callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for(auto& [key, chunk] : core.chunks) {
        chunk.create_vertices(core);
    }

    core.texture.bind(0);

    time_t start_time = get_time();
    core.last_time = get_time();

    while (core.game_running) {
        // -> logic

        time_t new_time = get_time();
        core.delta_time = new_time - core.last_time;

        if(core.delta_time >= 1000000000 / 60) {
            core.last_time = new_time;

            glfwPollEvents();

            core.game_loop();
        }

        core.game_running = !glfwWindowShouldClose(core.window);
    }

    core.save_game("saves\\world.bin");

    glfwTerminate();
}