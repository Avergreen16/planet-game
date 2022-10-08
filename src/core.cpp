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

struct Core {
    Camera camera;
    std::unordered_map<uint64_t, Chunk> loaded_chunks;
    std::vector<uint64_t> active_chunks;
    std::queue<uint64_t> chunk_update_queue;

    
    std::thread active_chunk_thread;
    std::thread chunk_update_thread;

    std::unordered_map<GLuint, bool> keymap = {
        {GLFW_KEY_W, false},
        {GLFW_KEY_A, false},
        {GLFW_KEY_S, false},
        {GLFW_KEY_D, false}
    };

    int& screen_width;
    int& screen_height;
    int& scale;
    
    bool& game_running;

    Core(glm::dvec2 camera_pos, bool& game_running, int& width, int& height, int& scale) : game_running(game_running), screen_width(width), screen_height(height), scale(scale) {
        this->camera.pos = camera_pos;

        // checks if the player moves between chunks, and if so, generates a new vector of chunk ids that corresponds to the player's new
        // location, so the player will always have chunks filling up their screen.
        active_chunk_thread = std::thread(
            [this]() {
                glm::ivec2 player_current_chunk(0x7fffffff, 0x7fffffff);
                glm::ivec2 view_area(2, 2); // chunks in each direction generated (needs to be dynamically calculated soon)
                while(this->game_running) {
                    glm::ivec2 updated_current_chunk(this->camera.pos.x / 16, this->camera.pos.y / 16);
                    
                    if(player_current_chunk != updated_current_chunk) {
                        player_current_chunk = updated_current_chunk;
                        std::vector<uint64_t> new_active_chunks;
                        for(int x = -view_area.x + updated_current_chunk.x; x <= view_area.x + updated_current_chunk.x; ++x) {
                            for(int y = -view_area.y + updated_current_chunk.y; y <= view_area.y + updated_current_chunk.y; ++y) {
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
        double move_dist = 0.000000005 * delta_time;
        glm::dvec2 move_vec(0, 0);
        if(keymap[GLFW_KEY_W]) {
            move_vec.y += 1;
        }
        if(keymap[GLFW_KEY_A]) {
            move_vec.x -= 1;
        }
        if(keymap[GLFW_KEY_S]) {
            move_vec.y -= 1;
        }
        if(keymap[GLFW_KEY_D]) {
            move_vec.x += 1;
        }

        if(!(move_vec.x == 0.0 && move_vec.y == 0.0)) camera.pos += glm::normalize(move_vec) * move_dist;
    }

    void render() {

    }
};