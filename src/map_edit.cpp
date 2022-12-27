#define USE_2D
#include "wrapper.cpp"
#include "fstream"
#include <chrono>
#include <deque>
#include <set>
#include <any>

time_t get_time() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

// shaders

const char *vs_grid = R"""(
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

const char *fs_grid = R"""(
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

const char *vs_hover = R"""(
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

const char *fs_hover = R"""(
#version 460 core
layout(location = 3) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = vec4(texelFetch(active_texture, ivec2(tex_coords), 0).rgb * 0.75 + vec3(0.25, 1.0, 1.0) * 0.25, 0.5);
}
)""";

const char *vs_select = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

layout(location = 0) uniform mat3 cam_matrix;

void main() {
    gl_Position = vec4((cam_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}
)""";

const char *fs_select = R"""(
#version 460 core
layout(location = 1) uniform float alpha;

out vec4 frag_color;

void main() {
    frag_color = vec4(0.25, 1.0, 1.0, alpha);
}
)""";

const char *vs_delete = R"""(
#version 460 core
layout(location = 0) in vec2 pos;

layout(location = 0) uniform mat3 pos_matrix;
layout(location = 1) uniform mat3 cam_matrix;

void main() {
    gl_Position = vec4((cam_matrix * pos_matrix * vec3(pos, 1.0)).xy, 0.5, 1.0);
}
)""";

const char *fs_delete = R"""(
#version 460 core
layout(location = 2) uniform float alpha;

out vec4 frag_color;

void main() {
    frag_color = vec4(1.0, 0.0, 0.0, alpha);
}
)""";

const char *vs_chunk = R"""(
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

const char *fs_chunk = R"""(
#version 460 core
layout(location = 2) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = texelFetch(active_texture, ivec2(tex_coords), 0);
}
)""";

const char *vs_paste = R"""(
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

const char *fs_paste = R"""(
#version 460 core
layout(location = 2) uniform sampler2D active_texture;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = vec4(texelFetch(active_texture, ivec2(tex_coords), 0).rgb * 0.75 + vec3(0.25, 1.0, 1.0) * 0.25, 0.5);
}
)""";

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
    glm::ivec2 loc;
    glm::ivec2 size;
};

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
    std::vector<Edit_tile> tiles;
};

struct Event_change_selection {
    glm::ivec2 bottom_left;
    std::set<glm::ivec2> selected_tiles;
};

enum key_status {
    PRESS_ON,
    PRESS_OFF,
    UP_ON,
    UP_OFF
};

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

    std::set<glm::ivec2, vec_comp> select_data;
    glm::ivec2 select_bottom_left = {0, 0};
    glm::ivec2 select_top_right = {0, 0};

    Buffer select_buffer;
    Buffer select_mod_buffer;
    bool select_mod_active = false;
    bool show_select_mod = false;
    bool show_select = false;

    bool reload_select_data_paste = false;

    std::set<glm::ivec2, vec_comp> select_data_paste;
    glm::ivec2 select_data_paste_bottom_left;

    std::vector<std::vector<int>> paste_data;


    bool select_drag = false;

    int tiles_in_paste = 0;
    bool reload_paste_buffer = true;
    Buffer paste_buffer;
    glm::ivec2 paste_offset = {0, 0};

    // used for undo/redo
    Event_edit_tiles new_edit_event;
    std::deque<Event> event_queue;
    int pos_in_queue = -1;

    std::map<int, key_status> keymap = { 
        {GLFW_KEY_LEFT_CONTROL, UP_OFF},
        {GLFW_KEY_LEFT_SHIFT, UP_OFF}
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

    void insert_edit_event() {
        if(new_edit_event.tiles.size() != 0) {
            if(event_queue.size() > pos_in_queue + 1) {
                event_queue.erase(event_queue.begin() + pos_in_queue + 1, event_queue.end());
            }
            pos_in_queue++;
            event_queue.push_back({new_edit_event, 0});

            new_edit_event.tiles.clear();
        }
    }

    void insert_action(Event& e) {
        if(event_queue.size() > pos_in_queue + 1) {
            event_queue.erase(event_queue.begin() + pos_in_queue + 1, event_queue.end());
        }
        pos_in_queue++;
        event_queue.push_back(e);
    }

    void update_select_buffer() {   
        std::vector<glm::vec2> vertex_vec;

        for(glm::ivec2 pos : select_data) {
            vertex_vec.push_back(glm::vec2(pos));
            vertex_vec.push_back(glm::vec2(pos + glm::ivec2{1, 0}));
            vertex_vec.push_back(glm::vec2(pos + glm::ivec2{0, 1}));
            vertex_vec.push_back(glm::vec2(pos + glm::ivec2{0, 1}));
            vertex_vec.push_back(glm::vec2(pos + glm::ivec2{1, 0}));
            vertex_vec.push_back(glm::vec2(pos + glm::ivec2{1, 1}));
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

    void add_select_data(bool reset_boundaries) {
        if(reset_boundaries || select_data.size() == 0) {
            select_bottom_left = bottom_left;
            select_top_right = top_right;
        } else {
            select_bottom_left = glm::ivec2(std::min(select_bottom_left.x, bottom_left.x), std::min(select_bottom_left.y, bottom_left.y));
            select_top_right = glm::ivec2(std::max(select_top_right.x, top_right.x), std::max(select_top_right.y, top_right.y));
        }

        bool update_buffer = false;

        for(int x = bottom_left.x; x <= top_right.x; x++) {
            for(int y = bottom_left.y; y <= top_right.y; y++) {
                if(select_data.insert({x, y}).second) {
                    update_buffer = true;
                }
            }
        }

        if(update_buffer) update_select_buffer();
        show_select = true;
    }

    void subtract_select_data() {
        if(select_data.size() != 0) {
            bool update_buffer = false;

            for(int x = bottom_left.x; x <= top_right.x; x++) {
                for(int y = bottom_left.y; y <= top_right.y; y++) {
                    if(select_data.erase({x, y})) {
                        update_buffer = true;
                    }
                }
            }

            if(select_data.size() != 0) {
                select_bottom_left.y = (*select_data.begin()).y;
                select_top_right.y = (*prev(select_data.end(), 1)).y;

                for(glm::ivec2 v : select_data) {
                    select_bottom_left.x = std::min(select_bottom_left.x, v.x);
                    select_top_right.x = std::max(select_top_right.x, v.x);
                }
            }

            if(update_buffer) update_select_buffer();
        }
    }

    void clear_select_data() {
        select_data.clear();
    }

    void update_paste_buffer() {
        std::vector<Vertex_0> vertex_vec(tiles_in_paste * 6);
        int i = 0;

        for(int x = 0; x < paste_data.size(); x++) {
            for(int y = 0; y < paste_data[0].size(); y++) {
                int tile = paste_data[x][y];
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

        reload_paste_buffer = false;
    }

    void move_selection(glm::ivec2 dist) {
        select_bottom_left += dist;
        select_top_right += dist;
        
        std::set<glm::ivec2, vec_comp> copied_data = select_data;
        select_data.clear();

        for(glm::ivec2 v : copied_data) {
            select_data.insert(v + dist);
        }

        update_select_buffer();
    }

    void load_paste_data();

    void insert_paste_data(glm::ivec2 paste_loc);

    void delete_selection(bool update_action);

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
                Tile_data &current_tile_data = core.tile_data[tile];
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
        while (!end) {
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
    }
    else {
        std::cout << "unable to open data.txt" << std::endl;
    }

    std::ifstream save;
    save.open("output\\save.txt");

    if(save.is_open()) {
        char string[64];
        bool end = false;
        while (!end) {
            save >> string;
            if(strcmp(string, "end") == 0) {
                end = true;
            } else if(strcmp(string, "chunk") == 0) {
                glm::ivec2 loc;
                save >> loc.x >> loc.y;

                chunks.insert({loc, Chunk(loc)});
                Chunk& chunk = chunks[loc];

                // save.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
    }
    else {
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

void Core::load_paste_data() {
    paste_data.clear();
    paste_data.resize(select_top_right.x - select_bottom_left.x + 1);
    tiles_in_paste = 0;

    int y_dif = select_top_right.y - select_bottom_left.y + 1;

    for(int i = 0; i < paste_data.size(); i++) {
        paste_data[i] = std::vector<int>(y_dif);
    }

    int count = 0;

    for(glm::ivec2 v : select_data) {
        int rel_x = v.x - select_bottom_left.x;
        glm::ivec2 chunk_key(v.x >> 4, v.y >> 4);
        if(chunks.contains(chunk_key)) {
            int tile = chunks[chunk_key].tiles[(v.x & 15) + ((v.y & 15) << 4)];
            if(tile)
                tiles_in_paste++;
            paste_data[rel_x][v.y - select_bottom_left.y] = tile;
        }
        count++;
    }

    reload_paste_buffer = true;
}

void Core::insert_paste_data(glm::ivec2 paste_loc) {
    std::set<Chunk*> edited_chunks;

    for(int x = 0; x < paste_data.size(); x++) {
        for(int y = 0; y < paste_data[0].size(); y++) {
            int set_tile = paste_data[x][y];
            if(set_tile) {
                glm::ivec2 insert_loc(paste_loc.x + x, paste_loc.y + y);
                glm::ivec2 chunk_key = insert_loc >> 4;

                if(!chunks.contains(chunk_key)) {
                    chunks.insert({chunk_key, Chunk(chunk_key)});
                }

                glm::ivec2 pos_in_chunk = insert_loc & 15;
                Chunk* chunk = &chunks[chunk_key];
                int &target_tile = chunk->tiles[pos_in_chunk.x + (pos_in_chunk.y << 4)];

                if(target_tile != set_tile) {
                    new_edit_event.tiles.push_back(Edit_tile{insert_loc, target_tile, set_tile});
                    target_tile = set_tile;

                    edited_chunks.insert(chunk);
                }
            }
        }
    }

    for(Chunk* c : edited_chunks) {
        c->create_vertices(*this);
    }

    insert_edit_event();
}

void Core::delete_selection(bool update_action) {
    std::set<Chunk*> edited_chunks;

    for(glm::ivec2 v : select_data) {
        glm::ivec2 chunk_key(v.x >> 4, v.y >> 4);

        if(chunks.contains(chunk_key)) {
            Chunk& chunk = chunks[chunk_key];
            int pos_in_chunk = (v.x & 15) + ((v.y & 15) << 4);
            int& tile = chunk.tiles[pos_in_chunk];
            if(tile) {
                new_edit_event.tiles.push_back(Edit_tile{v, tile, 0});
                tile = 0;
            }

            edited_chunks.insert(&chunk);
        }
    }

    for(Chunk* c : edited_chunks) {
        c->create_vertices(*this);
    }

    if(update_action) {
        insert_edit_event();
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
                    case SELECT: {
                        core.show_select = false;
                        core.show_select_mod = false;
                        core.clear_select_data();
                    }
                    case PASTE: {
                        if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_OFF) {
                            core.insert_paste_data(core.bottom_left);
                        } else {
                            core.insert_paste_data(core.paste_offset + core.mouse_tile);
                        }
                    }
                }
                core.active_edit_mode = SET;
            }
        } else if(key == GLFW_KEY_P) {
            if(core.active_mode == VIEW) {
                core.active_mode = EDIT;
            } else {
                core.active_mode = VIEW;
                if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_ON && core.new_edit_event.tiles.size() != 0) {
                    core.insert_edit_event();
                }
            }
        } else if(key == GLFW_KEY_F) {
            switch(core.active_edit_mode) {
                case SET:
                    core.active_edit_mode = FILL;
                    core.insert_edit_event();
                    break;
                case FILL:
                    core.active_edit_mode = SELECT;
                    break;
                case SELECT:
                    core.select_mod_active = false;
                    core.show_select_mod = false;
                    core.show_select = false;
                    core.clear_select_data();
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

        if(core.keymap[GLFW_KEY_LEFT_CONTROL] == PRESS_ON) {
            if(key == GLFW_KEY_Z) { // undo
                if(core.pos_in_queue != -1) {
                    Event& event = core.event_queue[core.pos_in_queue];
                    switch(event.id) {
                        case 0: {
                            Event_edit_tiles& e = *std::any_cast<Event_edit_tiles>(&event.contents);

                            std::set<Chunk*> edited_chunks;

                            for(auto t = --e.tiles.end(); t != --e.tiles.begin(); --t) {
                                Chunk& chunk = core.chunks[t->loc >> 4];
                                chunk.tiles[(t->loc.x & 15) + ((t->loc.y & 15) << 4)] = t->prev;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }
                        }
                        case 1: {
                            Event_change_selection& e = *std::any_cast<Event_change_selection>(&event.contents);

                            
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

                            for(Edit_tile& t : e.tiles) {
                                Chunk& chunk = core.chunks[t.loc >> 4];
                                chunk.tiles[(t.loc.x & 15) + ((t.loc.y & 15) << 4)] = t.set;
                                edited_chunks.insert(&chunk);
                            }

                            for(Chunk* c : edited_chunks) {
                                c->create_vertices(core);
                            }
                        }
                        case 1: {
                            Event_change_selection& e = *std::any_cast<Event_change_selection>(&event.contents);
                        }
                    }
                }
            } else if(key == GLFW_KEY_C) {
                if(core.show_select == true) {
                    core.load_paste_data();
                    core.select_data_paste = core.select_data;
                    core.select_data_paste_bottom_left = core.select_bottom_left;
                }
            } else if(key == GLFW_KEY_V) {
                if(core.active_mode == EDIT && core.paste_data.size() != 0) {
                    switch(core.active_edit_mode) {
                        case SET: {
                            core.insert_edit_event();
                            break;
                        }
                        case SELECT: {
                            core.select_mod_active = false;
                            core.show_select_mod = false;
                            core.show_select = false;
                            break;
                        }
                        case PASTE: {
                            if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == UP_OFF) {
                                core.insert_paste_data(core.select_bottom_left);
                            } else {
                                core.insert_paste_data(core.paste_offset + core.mouse_tile);
                            }
                            break;
                        }
                    }

                    core.paste_offset = -glm::ivec2{core.paste_data.size(), core.paste_data[0].size()} / 2;

                    core.select_data = core.select_data_paste;
                    core.select_bottom_left = core.select_data_paste_bottom_left;
                    core.move_selection(core.mouse_tile + core.paste_offset - core.select_bottom_left);
                    core.update_select_buffer();

                    core.active_edit_mode = PASTE;

                    if(core.reload_paste_buffer == true) {
                        core.update_paste_buffer();
                    }
                }
            } else if(key == GLFW_KEY_S) {
                core.save_game("output\\save.txt");
            }
        }
    }
    else if(action == GLFW_RELEASE) {
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
    }
    else if(y_offset < 0) {
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
    }
    else if(action == GLFW_RELEASE) {
        if(core.keymap_mouse.contains(button)) {
            core.keymap_mouse[button] = UP_ON;
        }
        if(button == GLFW_MOUSE_BUTTON_LEFT && core.active_mode == EDIT) {
            if(core.active_edit_mode == SET) {
                core.insert_edit_event();
            } else if(core.active_edit_mode == SELECT) {
                if(core.keymap[GLFW_KEY_LEFT_SHIFT] == PRESS_ON) {
                    core.add_select_data(false);
                } else if(core.keymap[GLFW_KEY_LEFT_CONTROL] == PRESS_ON) {
                    core.subtract_select_data();
                } else {
                    core.clear_select_data();
                    if(core.start_pos != core.end_pos) {
                        core.add_select_data(true);
                    } else {
                        core.update_select_buffer();
                    }
                }

                core.select_mod_active = false;
                core.show_select_mod = false;


            }
        }
    }
}

int main() {
    glm::mat3 identity_matrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

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
    core.select_mod_buffer.init();
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

    for(auto& [key, chunk] : core.chunks) {
        chunk.create_vertices(core);
    }

    time_t start_time = get_time();
    time_t time_container = get_time();

    while (core.game_running) {
        // -> logic

        time_t new_time = get_time();
        time_t delta_time = new_time - time_container;
        if(delta_time >= 1000000000 / 60) {
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
                            glm::ivec2 chunk_key = core.mouse_tile >> 4;

                            if(!core.chunks.contains(chunk_key)) {
                                core.chunks.insert({chunk_key, Chunk(chunk_key)});
                            }

                            Chunk* chunk = &core.chunks[chunk_key];
                            int &focus_tile = chunk->tiles[(core.mouse_tile.x & 15) + ((core.mouse_tile.y & 15) << 4)];

                            if(focus_tile != core.active_tile_id) {
                                core.new_edit_event.tiles.push_back(Edit_tile(core.mouse_tile, focus_tile, core.active_tile_id));

                                focus_tile = core.active_tile_id;
                                chunk->create_vertices(core);
                            }
                            break;
                        }

                        case FILL: {
                            core.mouse_tile;

                            glm::ivec2 chunk_key = core.mouse_tile >> 4;

                            if(!core.chunks.contains(chunk_key)) {
                                core.chunks.insert({chunk_key, Chunk(chunk_key)});
                            }

                            int focus_id = core.chunks[chunk_key].tiles[(core.mouse_tile.x & 15) + ((core.mouse_tile.y & 15) << 4)];

                            if(focus_id != core.active_tile_id) {
                                std::set<Chunk*> edited_chunks;

                                std::queue<glm::ivec2> queue;
                                queue.push(core.mouse_tile);
                                int counter = 0;
                                while (queue.size() != 0 && counter < 2113) {
                                    glm::ivec2 current_loc = queue.front();
                                    queue.pop();

                                    glm::ivec2 chunk_key = current_loc >> 4;
                                    if(!core.chunks.contains(chunk_key)) {
                                        core.chunks.insert({chunk_key, Chunk(chunk_key)});
                                    }
                                    Chunk* chunk = &core.chunks[chunk_key];

                                    int &tile = chunk->tiles[(current_loc.x & 15) + ((current_loc.y & 15) << 4)];

                                    if(tile == focus_id) {
                                        counter++;

                                        tile = core.active_tile_id;
                                        core.new_edit_event.tiles.push_back(Edit_tile(current_loc, focus_id, core.active_tile_id));

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
                                    c->create_vertices(core);
                                }

                                core.insert_edit_event();
                            }

                            core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                            break;
                        }

                        case SELECT: {
                            if(core.show_select && !core.select_mod_active && core.keymap[GLFW_KEY_LEFT_SHIFT] != PRESS_ON && core.keymap[GLFW_KEY_LEFT_CONTROL] != PRESS_ON && core.select_data.contains(core.mouse_tile)) {
                                core.select_drag = true;
                                core.paste_offset = core.select_bottom_left - core.mouse_tile;

                                //

                                core.load_paste_data();

                                core.delete_selection(false);

                                //

                                core.active_edit_mode = PASTE;

                                if(core.reload_paste_buffer == true) {
                                    core.update_paste_buffer();
                                }

                                core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                            } else {
                                core.start_pos = core.mouse_tile;
                                core.end_pos = core.mouse_tile;

                                core.bottom_left = glm::ivec2(std::min(core.start_pos.x, core.end_pos.x), std::min(core.start_pos.y, core.end_pos.y));
                                core.top_right = glm::ivec2(std::max(core.start_pos.x, core.end_pos.x), std::max(core.start_pos.y, core.end_pos.y));

                                core.select_mod_active = true;
                                core.show_select_mod = true;

                                core.update_select_mod_buffer();

                                core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                            }
                            break;
                        }

                        case PASTE: {
                            if(core.select_data.contains(core.mouse_tile)) {
                                core.paste_offset = core.select_bottom_left - core.mouse_tile;

                                core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] = PRESS_OFF;
                            } else {
                                core.insert_paste_data(core.select_bottom_left);

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
                            glm::ivec2 distance_moved = core.mouse_tile + core.paste_offset - core.select_bottom_left;

                            core.move_selection(distance_moved);

                            core.insert_paste_data(core.select_bottom_left);

                            //

                            core.select_drag = false;
                            core.active_edit_mode = SELECT;
                        } else {
                            glm::ivec2 distance_moved = core.mouse_tile + core.paste_offset - core.select_bottom_left;

                            core.move_selection(distance_moved);
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

            if(core.select_mod_active) {
                if(core.end_pos != core.mouse_tile) {
                    core.end_pos = core.mouse_tile;
                    core.bottom_left = glm::ivec2(std::min(core.start_pos.x, core.end_pos.x), std::min(core.start_pos.y, core.end_pos.y));
                    core.top_right = glm::ivec2(std::max(core.start_pos.x, core.end_pos.x), std::max(core.start_pos.y, core.end_pos.y));

                    core.update_select_mod_buffer();
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
                glm::mat3 chunk_matrix = glm::translate(identity_matrix, glm::vec2(chunk.pos << 4));
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
                } else if(core.active_edit_mode == SELECT) {
                    if(core.show_select_mod) {
                        core.select_mod_buffer.bind();

                        select_shader.use();
                        glUniformMatrix3fv(0, 1, false, &cam_matrix[0][0]);
                        glUniform1f(1, cos(double((time_container - start_time) / 1000000) * (0.003 * M_PI)) * 0.0625 + 0.25);

                        glDrawArrays(GL_TRIANGLES, 0, 6);
                    }
                    if(core.show_select) {
                        core.select_buffer.bind();

                        select_shader.use();
                        glUniformMatrix3fv(0, 1, false, &cam_matrix[0][0]);
                        glUniform1f(1, cos(double((time_container - start_time) / 1000000) * (0.003 * M_PI)) * 0.0625 + 0.25);

                        glDrawArrays(GL_TRIANGLES, 0, core.select_buffer.vertices);
                    }
                } else if(core.active_edit_mode == PASTE) {
                    paste_shader.use();

                    glm::vec2 location;
                    if(core.keymap_mouse[GLFW_MOUSE_BUTTON_LEFT] == PRESS_OFF) {
                        location = core.mouse_tile + core.paste_offset;
                    } else {
                        location = core.select_bottom_left;
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

                        glDrawArrays(GL_TRIANGLES, 0, core.select_buffer.vertices);
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
        }

        core.game_running = !glfwWindowShouldClose(window);
    }

    core.save_game("output\\save.txt");

    glfwTerminate();
}