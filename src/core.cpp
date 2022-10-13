#include "chunk.cpp"

uint64_t get_key(glm::ivec2 a) {
    return a.x + ((uint64_t)a.y << 32);
}

struct Camera {
    glm::dvec2 pos;

    Camera() = default;

    Camera(glm::dvec2 pos) {
        this->pos = pos;
    }

    glm::mat4 get_view_matrix() {
        return glm::lookAt(glm::vec3(pos, 1.0f), glm::vec3(pos, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

std::array<Vertex, 6> player_vertices = {
    Vertex{{-1, 0, 0}, {0, 0}, {31, 31}},
    Vertex{{1, 0, 0}, {32, 0}, {31, 31}},
    Vertex{{-1, 2, 0}, {0, 32}, {31, 31}},
    Vertex{{-1, 2, 0}, {0, 32}, {31, 31}},
    Vertex{{1, 0, 0}, {32, 0}, {31, 31}},
    Vertex{{1, 2, 0}, {32, 32}, {31, 31}}
};

enum direction:uint16_t {NORTH, EAST, SOUTH, WEST};

struct Core;

struct Player {
    glm::dvec2 position;
    glm::ivec2 active_texture = glm::ivec2(0, 0);
    Texture texture;
    Buffer buffer;

    direction dir_facing = SOUTH;

    double frame_timer = 0.0;
    
    void init(glm::dvec2 pos, Texture& tex) {
        this->position = pos;
        this->texture = tex;

        buffer.init();
        buffer.set_data(player_vertices.data(), 6 * sizeof(Vertex));
        buffer.set_attrib(0, 3, 7 * sizeof(float), 0);
        buffer.set_attrib(1, 2, 7 * sizeof(float), 3 * sizeof(float));
        buffer.set_attrib(2, 2, 7 * sizeof(float), 5 * sizeof(float));
    }

    void tick(Core& core, uint32_t delta);

    void render(Shader shader, glm::mat4& pv_mat) {
        glm::mat4 transform = glm::translate(glm::vec3(position, 0));

        shader.use();
        texture.bind(0, 3);

        buffer.bind();
        glUniformMatrix4fv(0, 1, GL_FALSE, &pv_mat[0][0]);
        glUniformMatrix4fv(1, 1, GL_FALSE, &transform[0][0]);
        glUniform2iv(2, 1, &active_texture[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

struct Core {
    Camera camera;
    Player player;
    glm::ivec2 mouse_pos;
    std::unordered_map<uint64_t, Chunk> loaded_chunks;
    std::vector<uint64_t> active_chunks;
    std::queue<uint64_t> chunk_update_queue;

    glm::ivec2 chunks_loaded = glm::ivec2(2, 2);
    bool reload_active_chunks = true;
    
    std::thread active_chunk_thread;
    std::thread chunk_update_thread;

    std::unordered_map<GLuint, bool> keymap = {
        {GLFW_KEY_W, false},
        {GLFW_KEY_A, false},
        {GLFW_KEY_S, false},
        {GLFW_KEY_D, false}
    };

    std::array<Texture, 2> textures;

    glm::ivec2& screen_size;
    int& scale;
    
    bool& game_running;

    Core(bool& game_running, glm::ivec2& screen_size, int& scale) : game_running(game_running), screen_size(screen_size), scale(scale) {
        mouse_pos = screen_size / 2;

        chunks_loaded = glm::ivec2(ceil((0.5 * screen_size.x) / (scale * 16)), ceil((0.5 * screen_size.y) / (scale * 16)));
    };

    void create_textures(std::array<const char*, 2>& input) {
        for(int i = 0; i < input.size(); ++i) {
            textures[i].load(input[i]);
        }
    }

    void init(glm::dvec2 camera_pos) {
        this->camera.pos = camera_pos;
        player.init(camera_pos, textures[1]);

        // checks if the player moves between chunks, and if so, generates a new vector of chunk ids that corresponds to the player's new
        // location, so the player will always have chunks filling up their screen.
        active_chunk_thread = std::thread(
            [this]() {
                glm::ivec2 player_current_chunk(0x7fffffff, 0x7fffffff);
                while(this->game_running) {
                    glm::ivec2 updated_current_chunk(floor(this->camera.pos.x / 16), floor(this->camera.pos.y / 16));
                    
                    if(player_current_chunk != updated_current_chunk || reload_active_chunks) {
                        reload_active_chunks = false;

                        player_current_chunk = updated_current_chunk;
                        std::vector<uint64_t> new_active_chunks;
                        for(int x = -chunks_loaded.x + updated_current_chunk.x; x <= chunks_loaded.x + updated_current_chunk.x; ++x) {
                            for(int y = -chunks_loaded.y + updated_current_chunk.y; y <= chunks_loaded.y + updated_current_chunk.y; ++y) {
                                glm::ivec2 pos(x, y);
                                uint64_t key = get_key(pos);
                                new_active_chunks.push_back(key);
                                if(!loaded_chunks.contains(key)) loaded_chunks.emplace(key, Chunk(pos));
                            }
                        }
                        this->active_chunks = new_active_chunks;
                    }
                }
            }
        );

        // pops a key off the update queue each loop and generates the mesh for the corresponding chunk
        chunk_update_thread = std::thread(
            [this]() {
                while(this->game_running) {
                    if(this->chunk_update_queue.size() != 0) {
                        uint64_t key = this->chunk_update_queue.front();
                        this->chunk_update_queue.pop();

                        this->loaded_chunks[key].generate_mesh();
                    }
                }
            }
        );
    }

    void math(uint32_t delta_time) {
        player.tick(*this, delta_time);
        glm::dvec2 mouse_offset = (mouse_pos - screen_size / 2) * 2;
        mouse_offset /= screen_size;

        camera.pos = player.position + glm::dvec2(0.0, 0.9375) + glm::dvec2(mouse_offset.x, -mouse_offset.y) * (48.0 / scale);
    }

    //void render() {

    //}
};

// needs to be moved so the core can be passed as parameter
void Player::tick(Core& core, uint32_t delta_time) {
    double seconds = 0.000000001 * delta_time;
    double move_dist = 5.0 * seconds;
    glm::dvec2 move_vec(0, 0);
    if(core.keymap[GLFW_KEY_W]) {
        move_vec.y += 1;
    }
    if(core.keymap[GLFW_KEY_A]) {
        move_vec.x -= 1;
    }
    if(core.keymap[GLFW_KEY_S]) {
        move_vec.y -= 1;
    }
    if(core.keymap[GLFW_KEY_D]) {
        move_vec.x += 1;
    }

    if(!(move_vec.x == 0.0 && move_vec.y == 0.0)) {
        if(active_texture.x == 0) {
            active_texture.x = 2;
        }
        
        position += glm::normalize(move_vec) * move_dist;
    
        if(move_vec.y > 0.0) {
            dir_facing = NORTH;
        } else if(move_vec.y < 0.0) {
            dir_facing = SOUTH;
        } else {
            if(move_vec.x > 0.0) {
                dir_facing = EAST;
            } else if(move_vec.x < 0.0) {
                dir_facing = WEST;
            }
        }

        frame_timer += seconds;
        if(frame_timer >= 0.175) {
            frame_timer -= 0.175;
            active_texture.x++;
            if(active_texture.x == 5) {
                active_texture.x = 1;
            }
        }
    } else {
        active_texture.x = 0;
        frame_timer = 0.0;
    }

    active_texture.y = dir_facing;
}