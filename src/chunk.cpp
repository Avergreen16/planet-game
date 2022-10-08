#include "wrapper.cpp"

std::array<glm::vec3, 4> tile_pos = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f)
};

struct Tile {
    glm::vec2 tex;
};

struct Chunk {
    glm::ivec2 pos;
    std::array<Tile, 256> tiles;
    Buffer buffer;
    unsigned int vertex_count;
    std::vector<Vertex> vertices;

    glm::mat4 transformation_mat;

    uint8_t vertex_status = 0;

    Chunk() = default;

    Chunk(glm::ivec2 pos) {
        this->pos = pos;
        this->transformation_mat = glm::translate(glm::vec3(pos.x * 16, pos.y * 16, 0));

        for(int x = 0; x < 16; x++) {
            for(int y = 0; y < 16; y++) {
                this->tiles[x + y * 16] = Tile{{0, (rand() % 4) * 16}};
            }
        }
    }

    void generate_mesh() {
        vertices.clear();
        vertices.resize(1536);
        for(int x = 0; x < 16; x++) {
            for(int y = 0; y < 16; y++) {
                int start_index = (x + y * 16) * 6;
                glm::vec3 position(x + pos.x * 16, y + pos.y * 16, 0);
                glm::vec2 tex = tiles[x + y * 16].tex;
                glm::vec2 tex_lim = tex + glm::vec2(15.0f, 15.0f);
                vertices[start_index] = Vertex(tile_pos[0] + position, tex + glm::vec2(0.0f, 0.0f), tex_lim);
                vertices[start_index + 1] = Vertex(tile_pos[1] + position, tex + glm::vec2(16.0f, 0.0f), tex_lim);
                vertices[start_index + 2] = Vertex(tile_pos[2] + position, tex + glm::vec2(0.0f, 16.0f), tex_lim);
                vertices[start_index + 3] = Vertex(tile_pos[2] + position, tex + glm::vec2(0.0f, 16.0f), tex_lim);
                vertices[start_index + 4] = Vertex(tile_pos[1] + position, tex + glm::vec2(16.0f, 0.0f), tex_lim);
                vertices[start_index + 5] = Vertex(tile_pos[3] + position, tex + glm::vec2(16.0f, 16.0f), tex_lim);
            }
        }
        vertex_status = 1;
    }

    void load_mesh() {
        buffer.init();
        buffer.set_data(vertices.data(), vertices.size() * sizeof(Vertex));
        buffer.set_attrib(0, 3, 7 * sizeof(float), 0);
        buffer.set_attrib(1, 2, 7 * sizeof(float), 3 * sizeof(float));
        buffer.set_attrib(2, 2, 7 * sizeof(float), 5 * sizeof(float));

        vertex_count = vertices.size();
        vertices.clear();
        vertex_status = 2;
    }

    void render(Shader& shader, Texture& texture) {
        shader.use();
        buffer.bind();
        texture.bind();
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    }
};