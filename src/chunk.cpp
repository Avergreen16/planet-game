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

    static Chunk* generate_chunk(siv::PerlinNoise& noise, glm::ivec2 pos) {
        Chunk* chunk_ptr = new Chunk;
        chunk_ptr->pos = pos;

        for(int x = 0; x < 16; x++) {
            for(int y = 0; y < 16; y++) {
                chunk_ptr->tiles[x + y * 16] = Tile{{0, (floor(noise.noise2D_01(float(x + pos.x) / 5, float(y + pos.y) / 5) * 4) * 16)}};
            }
        }

        return chunk_ptr;
    }

    void generate_mesh() {
        std::vector<Vertex> vertices;
        vertices.resize(1536);
        for(int x = 0; x < 16; x++) {
            for(int y = 0; y < 16; y++) {
                int start_index = (x + y * 16) * 6;
                glm::vec3 position(x + pos.x, y + pos.y, 0);
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

        buffer.init();
        buffer.set_data(vertices.data(), vertices.size() * sizeof(Vertex));
        buffer.set_attrib(0, 3, 5, 0);
        buffer.set_attrib(1, 2, 5, 3);
        vertex_count = vertices.size();
    }

    void render(Shader& shader, Texture& texture) {
        shader.use();
        buffer.bind();
        texture.bind();
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    }
};