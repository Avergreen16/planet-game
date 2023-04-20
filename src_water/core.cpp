#pragma once
#include "stuff.cpp"
#include "gui\gui.cpp"
#include "marching_cubes.cpp"

struct sphere_vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

std::vector<sphere_vertex> gen_sphere(float radius, int precision) {
    std::vector<glm::vec3> cube_face;
    float div = 2.0 / precision;
    for(int y = 0; y < precision; ++y) {
        for(int x = 0; x < precision; ++x) {
            cube_face.push_back({x * div - 1.0, y * div - 1.0, 1.0});
            cube_face.push_back({x * div + div - 1.0, y * div - 1.0, 1.0});
            cube_face.push_back({x * div + div - 1.0, y * div + div - 1.0, 1.0});
            cube_face.push_back({x * div - 1.0, y * div - 1.0, 1.0});
            cube_face.push_back({x * div + div - 1.0, y * div + div - 1.0, 1.0});
            cube_face.push_back({x * div - 1.0, y * div + div - 1.0, 1.0});
        }
    }

    std::vector<sphere_vertex> sphere_vector;

    // -x face
    for(glm::vec3 v : cube_face) {
        glm::vec3 n = {-v.z, -v.x, v.y};

        n = normalize(n);
        
        sphere_vector.push_back({n * radius, n, {atan2(n.y, n.x) / (M_PI * 2) + 0.5, atan2(n.z, sqrt(n.x * n.x + n.y * n.y)) / M_PI + 0.5}});
    }

    // +x face
    for(glm::vec3 v : cube_face) {
        glm::vec3 n = {v.z, v.x, v.y};

        n = normalize(n);
        
        sphere_vector.push_back({n * radius, n, {atan2(n.y, n.x) / (M_PI * 2) + 0.5, atan2(n.z, sqrt(n.x * n.x + n.y * n.y)) / M_PI + 0.5}});
    }

    // -y face
    for(glm::vec3 v : cube_face) {
        glm::vec3 n = {v.x, -v.z, v.y};

        n = normalize(n);
        
        sphere_vector.push_back({n * radius, n, {atan2(n.y, n.x) / (M_PI * 2) + 0.5, atan2(n.z, sqrt(n.x * n.x + n.y * n.y)) / M_PI + 0.5}});
    }

    // +y face
    for(glm::vec3 v : cube_face) {
        glm::vec3 n = {-v.x, v.z, v.y};

        n = normalize(n);
        
        sphere_vector.push_back({n * radius, n, {atan2(n.y, n.x) / (M_PI * 2) + 0.5, atan2(n.z, sqrt(n.x * n.x + n.y * n.y)) / M_PI + 0.5}});
    }

    // -z face
    for(glm::vec3 v : cube_face) {
        glm::vec3 n = {v.x, -v.y, -v.z};

        n = normalize(n);
        
        sphere_vector.push_back({n * radius, n, {atan2(n.y, n.x) / (M_PI * 2) + 0.5, atan2(n.z, sqrt(n.x * n.x + n.y * n.y)) / M_PI + 0.5}});
    }

    // +z face
    for(glm::vec3 v : cube_face) {
        glm::vec3 n = normalize(v);
        
        sphere_vector.push_back({n * radius, n, {atan2(n.y, n.x) / (M_PI * 2) + 0.5, atan2(n.z, sqrt(n.x * n.x + n.y * n.y)) / M_PI + 0.5}});
    }

    for(sphere_vertex& v : sphere_vector) {
        if(v.tex_coord.y < 0.000001) v.tex_coord.y = 0;
    }

    int i = 9 * precision * precision - precision - 2;

    sphere_vector[i * 3 + 2].tex_coord.x = sphere_vector[i * 3 + 1].tex_coord.x;
    sphere_vector[i * 3 + 4].tex_coord.x = sphere_vector[i * 3 + 3].tex_coord.x;

    i += 2;

    sphere_vector[i * 3 + 2] = sphere_vector[i * 3 + 5];
    sphere_vector[i * 3 + 3] = sphere_vector[i * 3 + 1];

    sphere_vector[i * 3 + 2].tex_coord.x = sphere_vector[i * 3 + 1].tex_coord.x;
    sphere_vector[i * 3 + 5].tex_coord.x = sphere_vector[i * 3 + 4].tex_coord.x;

    i += 2 * precision - 2;

    sphere_vector[i * 3 + 2] = sphere_vector[i * 3 + 5];
    sphere_vector[i * 3 + 3] = sphere_vector[i * 3 + 1];

    sphere_vector[i * 3 + 3].tex_coord.x = sphere_vector[i * 3 + 5].tex_coord.x;
    sphere_vector[i * 3 + 1].tex_coord.x = sphere_vector[i * 3 + 0].tex_coord.x;

    i += 2;
    
    sphere_vector[i * 3 + 0].tex_coord.x = sphere_vector[i * 3 + 2].tex_coord.x;
    sphere_vector[i * 3 + 3].tex_coord.x = sphere_vector[i * 3 + 5].tex_coord.x;


    i = 11 * precision * precision - precision - 2;

    sphere_vector[i * 3 + 2].tex_coord.x = sphere_vector[i * 3 + 1].tex_coord.x;
    sphere_vector[i * 3 + 4].tex_coord.x = sphere_vector[i * 3 + 3].tex_coord.x;

    i += 2;

    sphere_vector[i * 3 + 2] = sphere_vector[i * 3 + 5];
    sphere_vector[i * 3 + 3] = sphere_vector[i * 3 + 1];

    sphere_vector[i * 3 + 2].tex_coord.x = sphere_vector[i * 3 + 1].tex_coord.x;
    sphere_vector[i * 3 + 5].tex_coord.x = sphere_vector[i * 3 + 4].tex_coord.x;

    i += 2 * precision - 2;

    sphere_vector[i * 3 + 2] = sphere_vector[i * 3 + 5];
    sphere_vector[i * 3 + 3] = sphere_vector[i * 3 + 1];

    sphere_vector[i * 3 + 3].tex_coord.x = sphere_vector[i * 3 + 5].tex_coord.x;
    sphere_vector[i * 3 + 1].tex_coord.x = sphere_vector[i * 3 + 0].tex_coord.x;

    i += 2;
    
    sphere_vector[i * 3 + 0].tex_coord.x = sphere_vector[i * 3 + 2].tex_coord.x;
    sphere_vector[i * 3 + 3].tex_coord.x = sphere_vector[i * 3 + 5].tex_coord.x;



    for(int i = 0; i < sphere_vector.size(); i += 3) {
        sphere_vertex* v0 = &sphere_vector[i];
        sphere_vertex* v1 = &sphere_vector[i + 1];
        sphere_vertex* v2 = &sphere_vector[i + 2];

        sphere_vertex* empty;

        /*if(v0->tex_coord.y == 0 || v0->tex_coord.y == 1) {
            sphere_vertex* a = v0;
            v0 = v1;
            v1 = a;
            
            glm::vec2 diff_2 = v2->tex_coord - v0->tex_coord;

            if(diff_2.x > 0.5) {
                v2->tex_coord.x -= 1;
            } else if(diff_2.x < -0.5) {
                v2->tex_coord.x += 1;
            }
        } else if(v1->tex_coord.y == 0 || v1->tex_coord.y == 1) {
            glm::vec2 diff_2 = v2->tex_coord - v0->tex_coord;

            if(diff_2.x > 0.5) {
                v2->tex_coord.x -= 1;
            } else if(diff_2.x < -0.5) {
                v2->tex_coord.x += 1;
            }
        } else if(v2->tex_coord.y == 0 || v2->tex_coord.y == 1) {
            glm::vec2 diff_1 = v1->tex_coord - v0->tex_coord;

            if(diff_1.x > 0.5) {
                v1->tex_coord.x -= 1;
            } else if(diff_1.x < -0.5) {
                v1->tex_coord.x += 1;
            }
        } else {
            glm::vec2 diff_1 = v1->tex_coord - v0->tex_coord;
            glm::vec2 diff_2 = v2->tex_coord - v0->tex_coord;

            if(diff_1.x > 0.5) {
                v1->tex_coord.x -= 1;
            } else if(diff_1.x < -0.5) {
                v1->tex_coord.x += 1;
            }
            if(diff_2.x > 0.5) {
                v2->tex_coord.x -= 1;
            } else if(diff_2.x < -0.5) {
                v2->tex_coord.x += 1;
            }
        }*/

        if(v0->tex_coord.y == 0 || v0->tex_coord.y == 1) {
            sphere_vertex* a = v0;
            v0 = v1;
            v1 = a;
        }

        glm::vec2 diff_1 = v1->tex_coord - v0->tex_coord;
        glm::vec2 diff_2 = v2->tex_coord - v0->tex_coord;

        if(diff_1.x > 0.5) {
            v1->tex_coord.x -= 1;
        } else if(diff_1.x < -0.5) {
            v1->tex_coord.x += 1;
        }
        if(diff_2.x > 0.5) {
            v2->tex_coord.x -= 1;
        } else if(diff_2.x < -0.5) {
            v2->tex_coord.x += 1;
        }
    }

    return std::move(sphere_vector);
}

struct Planet {
    glm::vec3 position;
    float radius;
    float mass;
    Buffer buffer;
    int surface_index;
    glm::vec3 color;
    bool use_color = false;
    bool is_star = false;

    glm::dvec3 velocity = {0, 0, 0};
    glm::dvec3 acceleration = {0, 0, 0};

    float rotation;

    Planet(const Planet& p) = delete;
    Planet(Planet&& p) noexcept = default;
    Planet(glm::vec3 pos, float rad, float m, float rot, int s_index, bool u_color = false, glm::vec3 col = {0, 0, 0}, bool i_star = false) {
        position = pos;
        radius = rad;
        mass = m;
        use_color = u_color;
        color = col;
        is_star = i_star;
        surface_index = s_index;
        rotation = rot;
    }
};

struct Space_core {
    std::vector<Planet> planets;
    std::vector<Texture> textures;
    Shader planet_shader;
    Shader planet_shader_solid;

    Framebuffer<1> framebuffer;

    std::thread thread;

    glm::vec3 light_pos;
    double G = 0.0000000005;

    bool sim_active = false;
    double sim_speed = 1;

    std::time_t start_time;
    std::time_t t;
    
    void loop() {
        t = get_time();
        start_time = t;
        while(true) {
            std::time_t new_time = get_time();
            double delta_time = double(new_time - t);
            t = new_time;

            if(sim_active) {
                for(int i = 0; i < planets.size() - 1; ++i) {
                    for(int j = i + 1; j < planets.size(); ++j) {
                        glm::dvec3 difference = planets[i].position - planets[j].position;
                        glm::dvec3 norm = normalize(difference);
                        double dist = glm::length(difference);

                        double x = G / (dist * dist);

                        planets[i].acceleration -= norm * (x * planets[j].mass);
                        planets[j].acceleration += norm * (x * planets[i].mass);
                    }
                }

                std::cout << delta_time << "\n";

                for(Planet& p : planets) {
                    p.velocity += p.acceleration * delta_time * sim_speed;
                    p.acceleration = {0, 0, 0};
                    p.position += p.velocity * delta_time * sim_speed;
                }
            }
        }
    }

    void orbit(Planet& a, Planet& b) {
        glm::dvec3 dist = b.position - a.position;
        glm::dvec3 dir = normalize(cross(dist, {0, 0, 1}));
        a.velocity = dir * sqrt(G * b.mass / glm::length(dist)) + b.velocity;
    }

    void init() {
        for(int i = 0; i < 5; ++i) {
            textures.push_back(Texture());
        }
        textures[0].load("res/planet_surfaces/gas_giant.png");
        textures[1].load("res/planet_surfaces/ice_giant.png");
        textures[2].load("res/planet_surfaces/forest.png");
        textures[3].load("res/planet_surfaces/desert.png");
        textures[4].load("res/planet_surfaces/icy.png");

        planet_shader.compile("res/shaders/planet.vs", "res/shaders/planet.fs");
        planet_shader_solid.compile("res/shaders/planet_solid.vs", "res/shaders/planet_solid.fs");

        planets.push_back(Planet(glm::vec3{0, 0, 0}, 20.0f, 2000.0f, 87, -1, true, {1.0, 0.8, 0.5}, true));
        planets.push_back(Planet(normalize(glm::vec3{160, 1600, 0}) * 1600.0f, 6.0f, 25.0f, 43, 0));
        planets.push_back(Planet(planets[1].position + normalize(glm::vec3{4, 20, 0}) * 25.0f, 0.5f, 0.25f, 13, 3));
        planets.push_back(Planet(normalize(glm::vec3{600, -100, 0}) * 650.0f, 1.0f, 1.0f, 48, 2));
        planets.push_back(Planet(normalize(glm::vec3{350, 100, 0}) * 300.0f, 5.0f, 15.0f, 276, 1));
        planets.push_back(Planet(planets[3].position + normalize(glm::vec3{3, -4.5, 0}) * 7.0f, 0.2f, 0.1f, 24, 4));
        planets.push_back(Planet(normalize(glm::vec3{600, 100, 0}) * 1000.0f, 1.7f, 2.5f, 196, 4));

        orbit(planets[1], planets[0]);
        orbit(planets[2], planets[1]);
        orbit(planets[4], planets[0]);
        orbit(planets[3], planets[0]);
        orbit(planets[5], planets[3]);
        orbit(planets[6], planets[0]);

        light_pos = planets[0].position;
        
        for(Planet& p : planets) {
            auto v = gen_sphere(p.radius, 8);

            p.buffer.init();
            p.buffer.set_data(v.data(), v.size(), sizeof(v[0]));
            p.buffer.set_attrib(0, 3, sizeof(float) * 8, 0);
            p.buffer.set_attrib(1, 3, sizeof(float) * 8, sizeof(float) * 3);
            p.buffer.set_attrib(2, 2, sizeof(float) * 8, sizeof(float) * 6);
        }

        thread = std::thread(
            [this]() {
                loop();
            }
        );
    }

    void draw(glm::mat4 view_mat, glm::mat4 proj_mat);
};

const glm::ivec3 region = {16, 16, 6};

const int factor = 128;
const float freq = 1.0 / factor;

glm::vec3 apply_matrix(glm::vec3& vec, glm::mat4& mat) {
    return (mat * glm::vec4{vec, 1.0}).xyz();
}

struct comparator {
    bool const operator()(const glm::ivec3& a, const glm::ivec3& b) const {
        return a.x > b.x || (a.x == b.x && (a.y > b.y || (a.y == b.y && a.z < b.z))); 
    }
};

struct comparator_nearest_chunk {
    bool const operator()(const glm::ivec3& a, const glm::ivec3& b) const;
};

struct ivec3_hash {
    std::size_t operator()(const glm::ivec3& a) const {
        return std::hash<int>()(a.x) ^ std::hash<int>()(a.y) ^ std::hash<int>()(a.z);
    }
};

struct chunk_vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex_coord;
    glm::vec2 tile_coord;
};

int mod(int x, int y) {
    return x - y * floor((float)x / y);
}

glm::ivec3 mod(glm::ivec3 x, int y) {
    return {x.x - y * floor((float)x.x / y), x.y - y * floor((float)x.y / y), x.z - y * floor((float)x.z / y)};
}

std::unordered_map<uint16_t, glm::vec2> tile_coord = {
    {1, {0, 1}},
    {2, {1, 1}},
    {3, {1, 0}},
    {4, {2, 1}}
};

struct Chunk {
    glm::ivec3 index;
    std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x20>, 0x20>, 0x20>> data = std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x20>, 0x20>, 0x20>>(new std::array<std::array<std::array<uint16_t, 0x20>, 0x20>, 0x20>);
    Buffer buffer;
    uint8_t status = 0;
    std::vector<chunk_vertex> vertices;

    void generate();

    void load_neighbors();

    void create_mesh();

    void load_buffers();

    Chunk(glm::ivec3 i) {
        index = i;
    }
    Chunk() = default;
    Chunk(Chunk&& c) noexcept = default;
};

bool check() {
    std::cout << "check" << "\n";
    return true;
}


enum dir:uint8_t{NUL, NORTH, EAST, SOUTH, WEST, UP, DOWN};

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    glm::ivec2 viewport_size = {800, 600};
    //glm::ivec2 monitor_size = {0, 0};
    glm::ivec2 cursor_pos = {0, 0};
    bool cursor_hidden = false;

    glm::mat3 screen_matrix;

    glm::vec3 view_pos = {0, 0, 0};
    glm::vec3 view_dir = {0, 1, 0};
    glm::vec3 up_dir = {0, 0, 1};
    glm::vec3 z_dir = {0, 0, 1};
    double move_speed = 8;
    uint8_t dir_enum = NUL;

    Shader grid_shader;
    Shader chunk_shader;
    Shader screen_shader;
    Shader screen_shader_solid;
    Shader cube_shader;

    Texture tex;

    uint64_t time;
    double frame_time;
    double tick_time;
    uint16_t frame_count = 0;

    Gui_core gui_core;
    Space_core space_core;
    Framebuffer<1> gui_framebuffer;

    glm::ivec3 selected_block;
    bool block_selected = false;

    std::unordered_map<GLuint, bool> key_map = {
        {GLFW_KEY_RIGHT_SHIFT, false},
        {GLFW_KEY_W, false},
        {GLFW_KEY_S, false},
        {GLFW_KEY_A, false},
        {GLFW_KEY_D, false},
        {GLFW_KEY_Q, false},
        {GLFW_KEY_E, false},
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_LEFT_SHIFT, false},
        {GLFW_KEY_MINUS, false},
        {GLFW_KEY_EQUAL, false},
        {GLFW_KEY_LEFT_CONTROL, false}
    };

    std::unordered_map<GLuint, bool> mouse_button_map = {
        {GLFW_MOUSE_BUTTON_LEFT, false},
        {GLFW_MOUSE_BUTTON_RIGHT, false}
    };

    std::unordered_map<glm::ivec3, Chunk, ivec3_hash> chunks;
    //std::map<glm::ivec3, Chunk, decltype(&comparator)> chunks = std::map<glm::ivec3, Chunk, decltype(&comparator)>(comparator);
    std::set<glm::ivec3, comparator> block_updates;
    std::set<glm::ivec3, comparator> changed_chunks;
    std::priority_queue<glm::ivec3, std::vector<glm::ivec3>, comparator_nearest_chunk> chunk_gen_queue;
    std::vector<glm::ivec3> chunk_buffer_updates;
    
    std::thread chunk_thread;
    std::mutex chunk_allocate_thread_mutex;
    std::mutex chunk_allocate_main_mutex;
    std::mutex gen_queue_mutex;
    std::mutex buffer_update_mutex;

    glm::ivec3 current_index = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
    glm::ivec3 current_coordinate = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

    FastNoise::SmartNode<FastNoise::Perlin> fnperlin = FastNoise::New<FastNoise::Perlin>();
    FastNoise::SmartNode<FastNoise::FractalFBm> fnfractal = FastNoise::New<FastNoise::FractalFBm>();

    void game_loop();

    void init();

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
        space_core.framebuffer.resize(viewport_size);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        std::get<1>(gui_core.widgets[3]).box.position = viewport_size / 2 + glm::ivec2{-5, -9};

        screen_matrix = glm::translate(glm::scale(identity_matrix_3, glm::vec2(2.0 / viewport_size.x, 2.0 / viewport_size.y)), glm::vec2(-viewport_size / 2));
        
        game_loop();
    }

    void cursor_pos_update(double xpos, double ypos) {
        glm::ivec2 new_cursor_pos = {xpos, screen_size.y - ypos - 1};
        glm::dvec2 difference = new_cursor_pos - cursor_pos;

        if(cursor_hidden) {
            glm::mat4 rotate_matrix_x = glm::rotate(identity_matrix_4, float(2 * M_PI * 0x0.008p0 * -difference.x), up_dir);
            glm::mat4 rotate_matrix_y = glm::rotate(identity_matrix_4, float(2 * M_PI * 0x0.008p0 * difference.y), glm::cross(view_dir, up_dir));

            view_dir = rotate_matrix_y * (rotate_matrix_x * glm::vec4(view_dir, 1.0));
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
            if(key_map[GLFW_KEY_LEFT_CONTROL]) {
                space_core.sim_speed *= 2;
            } else {
                move_speed *= 2;
            }
        } else if(yoffset < 0) {
            if(key_map[GLFW_KEY_LEFT_CONTROL]) {
                space_core.sim_speed /= 2;
            } else {
                move_speed /= 2;
            }
        }
    }
};

Core core;

bool const comparator_nearest_chunk::operator()(const glm::ivec3& a, const glm::ivec3& b) const {
    glm::ivec3 abs_diff_a = glm::abs(a - core.current_index);   
    glm::ivec3 abs_diff_b = glm::abs(b - core.current_index);
    int sum_a = abs_diff_a.x + abs_diff_a.y + abs_diff_a.z;
    int sum_b = abs_diff_b.x + abs_diff_b.y + abs_diff_b.z;

    return sum_a > sum_b;//abs_diff_a.x < abs_diff_b.x || (abs_diff_a.x == abs_diff_b.x && (abs_diff_a.y < abs_diff_b.y || (abs_diff_a.y == abs_diff_b.y && abs_diff_a.z < abs_diff_b.z)));
}

glm::mat4 rotation_mat = glm::inverse(glm::rotate(identity_matrix_4, float(1.0 / 4 * M_PI), glm::normalize(glm::vec3(0, 0, 1))));

const float fract_x = (256.0 / 80), fract_y = (256.0 / 40), fract_z = (256.0 / 60);
//const float fract_x = 1, fract_y = 1, fract_z = 1;

void Chunk::generate() {
    glm::ivec3 chunk_vec = index * 0x20;
    std::array<float, 0x8000> result;
    core.fnfractal->GenUniformGrid3D(result.data(), index.x * 0x20, index.y * 0x20, index.z * 0x20, 0x20, 0x20, 0x20, freq, 0x1B);

    for(float x = 0; x < 0x20; ++x) {
        for(float y = 0; y < 0x20; ++y) {
            for(float z = 0; z < 0x20; ++z) {
                uint16_t vox = (result[x + y * 0x20 + z * 0x400] * 48 + 32 - (chunk_vec.z + z) > 0.0f) ? ((rand() > 0x2800) ? 1 : 2) : 0;
                if(vox != 0 && chunk_vec.z + z > 32) vox = 3;
                else if(vox == 0 && chunk_vec.z + z < 32) vox = 4;
                (*data.get())[x][y][z] = vox;
            }
        }
    }
    // std::max(std::max(abs(x + chunk_vec.x), abs(y + chunk_vec.y)), abs(z + chunk_vec.z))
    // result[x + y * 0x20 + z * 0x400] * 6 + 0x80 - glm::length(glm::vec3(x, y, z) + glm::vec3(chunk_vec)) > 0.0f
}

void Chunk::load_neighbors() {
    /*
    if(!core.chunks.contains(index + glm::ivec3{-1, 0, 0})) {
        core.chunks.emplace(index + glm::ivec3{-1, 0, 0}, Chunk(index + glm::ivec3{-1, 0, 0}));
        core.chunks[index + glm::ivec3{-1, 0, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{1, 0, 0})) {
        core.chunks.emplace(index + glm::ivec3{1, 0, 0}, Chunk(index + glm::ivec3{1, 0, 0}));
        core.chunks[index + glm::ivec3{1, 0, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, -1, 0})) {
        core.chunks.emplace(index + glm::ivec3{0, -1, 0}, Chunk(index + glm::ivec3{0, -1, 0}));
        core.chunks[index + glm::ivec3{0, -1, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 1, 0})) {
        core.chunks.emplace(index + glm::ivec3{0, 1, 0}, Chunk(index + glm::ivec3{0, 1, 0}));
        core.chunks[index + glm::ivec3{0, 1, 0}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 0, -1})) {
        core.chunks.emplace(index + glm::ivec3{0, 0, -1}, Chunk(index + glm::ivec3{0, 0, -1}));
        core.chunks[index + glm::ivec3{0, 0, -1}].generate();
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 0, 1})) {
        core.chunks.emplace(index + glm::ivec3{0, 0, 1}, Chunk(index + glm::ivec3{0, 0, 1}));
        core.chunks[index + glm::ivec3{0, 0, 1}].generate();
    }
    */
   std::vector<glm::ivec3> keys;
    
    if(!core.chunks.contains(index + glm::ivec3{1, 0, 0})) {
        keys.push_back(index + glm::ivec3{1, 0, 0});
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 1, 0})) {
        keys.push_back(index + glm::ivec3{0, 1, 0});
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 0, 1})) {
        keys.push_back(index + glm::ivec3{0, 0, 1});
    }
    if(!core.chunks.contains(index + glm::ivec3{1, 1, 0})) {
        keys.push_back(index + glm::ivec3{1, 1, 0});
    }
    if(!core.chunks.contains(index + glm::ivec3{1, 0, 1})) {
        keys.push_back(index + glm::ivec3{1, 0, 1});
    }
    if(!core.chunks.contains(index + glm::ivec3{0, 1, 1})) {
        keys.push_back(index + glm::ivec3{0, 1, 1});
    }
    if(!core.chunks.contains(index + glm::ivec3{1, 1, 1})) {
        keys.push_back(index + glm::ivec3{1, 1, 1});
    }

    core.chunk_allocate_thread_mutex.lock();
    for(glm::ivec3 k : keys) {
        core.chunks.insert({k, Chunk(k)});
    }
    core.chunk_allocate_thread_mutex.unlock();

    for(glm::ivec3 k : keys) {
        core.chunks[k].generate();
    }
}

void Chunk::create_mesh() {
    load_neighbors();

    auto& d = (*data.get());
    auto& nx = (*core.chunks[index + glm::ivec3{1, 0, 0}].data.get());
    auto& ny = (*core.chunks[index + glm::ivec3{0, 1, 0}].data.get());
    auto& nz = (*core.chunks[index + glm::ivec3{0, 0, 1}].data.get());
    auto& nxy = (*core.chunks[index + glm::ivec3{1, 1, 0}].data.get());
    auto& nxz = (*core.chunks[index + glm::ivec3{1, 0, 1}].data.get());
    auto& nyz = (*core.chunks[index + glm::ivec3{0, 1, 1}].data.get());
    auto& nxyz = (*core.chunks[index + glm::ivec3{1, 1, 1}].data.get());

    for(uint8_t x = 0; x < 0x20; ++x) {
        for(uint8_t y = 0; y < 0x20; ++y) {
            for(uint8_t z = 0; z < 0x20; ++z) {
                glm::vec3 pos = {x, y, z};
                uint16_t id = d[x][y][z];

                std::array<uint16_t, 8> voxels;

                voxels[0] = id;

                if(x == 0x1F) {
                    if(y == 0x1F) {
                        if(z == 0x1F) {
                            voxels[1] = nx[0][y][z];
                            voxels[2] = nxz[0][y][0];
                            voxels[3] = nz[x][y][0];
                            voxels[4] = ny[x][0][z];
                            voxels[5] = nxy[0][0][z];
                            voxels[6] = nxyz[0][0][0];
                            voxels[7] = nyz[x][0][0];
                        } else {
                            voxels[1] = nx[0][y][z];
                            voxels[2] = nx[0][y][z + 1];
                            voxels[3] = d[x][y][z + 1];
                            voxels[4] = ny[x][0][z];
                            voxels[5] = nxy[0][0][z];
                            voxels[6] = nxy[0][0][z + 1];
                            voxels[7] = ny[x][0][z + 1];
                        }
                    } else if(z == 0x1F) {
                        voxels[1] = nx[0][y][z];
                        voxels[2] = nxz[0][y][0];
                        voxels[3] = nz[x][y][0];
                        voxels[4] = d[x][y + 1][z];
                        voxels[5] = nx[0][y + 1][z];
                        voxels[6] = nxz[0][y + 1][0];
                        voxels[7] = nz[x][y + 1][0];
                    } else {
                        voxels[1] = nx[0][y][z];
                        voxels[2] = nx[0][y][z + 1];
                        voxels[3] = d[x][y][z + 1];
                        voxels[4] = d[x][y + 1][z];
                        voxels[5] = nx[0][y + 1][z];
                        voxels[6] = nx[0][y + 1][z + 1];
                        voxels[7] = d[x][y + 1][z + 1];
                    }
                } else if(y == 0x1F) {
                    if(z == 0x1F) {
                        voxels[1] = d[x + 1][y][z];
                        voxels[2] = nz[x + 1][y][0];
                        voxels[3] = nz[x][y][0];
                        voxels[4] = ny[x][0][z];
                        voxels[5] = ny[x + 1][0][z];
                        voxels[6] = nyz[x + 1][0][0];
                        voxels[7] = nyz[x][0][0];
                    } else {
                        voxels[1] = d[x + 1][y][z];
                        voxels[2] = d[x + 1][y][z + 1];
                        voxels[3] = d[x][y][z + 1];
                        voxels[4] = ny[x][0][z];
                        voxels[5] = ny[x + 1][0][z];
                        voxels[6] = ny[x + 1][0][z + 1];
                        voxels[7] = ny[x][0][z + 1];
                    }
                } else if(z == 0x1F) {
                    voxels[1] = d[x + 1][y][z];
                    voxels[2] = nz[x + 1][y][0];
                    voxels[3] = nz[x][y][0];
                    voxels[4] = d[x][y + 1][z];
                    voxels[5] = d[x + 1][y + 1][z];
                    voxels[6] = nz[x + 1][y + 1][0];
                    voxels[7] = nz[x][y + 1][0];
                } else {
                    voxels[1] = d[x + 1][y][z];
                    voxels[2] = d[x + 1][y][z + 1];
                    voxels[3] = d[x][y][z + 1];
                    voxels[4] = d[x][y + 1][z];
                    voxels[5] = d[x + 1][y + 1][z];
                    voxels[6] = d[x + 1][y + 1][z + 1];
                    voxels[7] = d[x][y + 1][z + 1];
                }

                /*if(voxels[0] == 0) {
                    if(voxels[1] != 0) {
                        glm::vec3 normal = glm::cross(corners[triangle_table_cube[0][0]] - corners[triangle_table_cube[0][2]], corners[triangle_table_cube[0][1]] - corners[triangle_table_cube[0][2]]);
                            
                        glm::mat4 matrix = identity_matrix_4;
                        if(normal != core.z_dir) {
                            if(normal == -core.z_dir) {
                                matrix = rot_z_4;
                            } else {
                                glm::vec3 cross = glm::normalize(glm::cross(normal, core.z_dir));
                                float angle = glm::acos(dot(normal, core.z_dir));
                                matrix = glm::rotate(angle, cross);

                                matrix = glm::lookAt(glm::vec3(0, 0, 0), -normal, core.z_dir);
                            }
                        }

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + glm::vec3(1, 0, 0) + corners[triangle_table_cube[0][k * 3 + l]];
                                glm::vec2 tex_coord = (matrix * glm::vec4(position, 1.0)).xy();
                                position -= 0.5f;
                                vertices.push_back({position, normal, tex_coord, tile_coord[voxels[1]]});
                            }   
                        }
                    }
                    if(voxels[4] != 0) {
                        glm::vec3 normal = glm::cross(corners[triangle_table_cube[1][0]] - corners[triangle_table_cube[1][2]], corners[triangle_table_cube[1][1]] - corners[triangle_table_cube[1][2]]);
                            
                        glm::mat4 matrix = identity_matrix_4;
                        if(normal != core.z_dir) {
                            if(normal == -core.z_dir) {
                                matrix = rot_z_4;
                            } else {
                                glm::vec3 cross = glm::normalize(glm::cross(normal, core.z_dir));
                                float angle = glm::acos(dot(normal, core.z_dir));
                                matrix = glm::rotate(angle, cross);

                                matrix = glm::lookAt(glm::vec3(0, 0, 0), -normal, core.z_dir);
                            }
                        }

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + glm::vec3(0, 1, 0) + corners[triangle_table_cube[1][k * 3 + l]];
                                glm::vec2 tex_coord = (matrix * glm::vec4(position, 1.0)).xy();
                                position -= 0.5f;
                                vertices.push_back({position, normal, tex_coord, tile_coord[voxels[4]]});
                            }   
                        }
                    }
                    if(voxels[3] != 0) {
                        glm::vec3 normal = glm::cross(corners[triangle_table_cube[2][0]] - corners[triangle_table_cube[2][2]], corners[triangle_table_cube[2][1]] - corners[triangle_table_cube[2][2]]);
                            
                        glm::mat4 matrix = identity_matrix_4;
                        if(normal != core.z_dir) {
                            if(normal == -core.z_dir) {
                                matrix = rot_z_4;
                            } else {
                                glm::vec3 cross = glm::normalize(glm::cross(normal, core.z_dir));
                                float angle = glm::acos(dot(normal, core.z_dir));
                                matrix = glm::rotate(angle, cross);

                                matrix = glm::lookAt(glm::vec3(0, 0, 0), -normal, core.z_dir);
                            }
                        }

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + glm::vec3(0, 0, 1) + corners[triangle_table_cube[2][k * 3 + l]];
                                glm::vec2 tex_coord = (matrix * glm::vec4(position, 1.0)).xy();
                                position -= 0.5f;
                                vertices.push_back({position, normal, tex_coord, tile_coord[voxels[3]]});
                            }   
                        }
                    }
                } else {
                    if(voxels[1] == 0) {
                        glm::vec3 normal = glm::cross(corners[triangle_table_cube[3][0]] - corners[triangle_table_cube[3][2]], corners[triangle_table_cube[3][1]] - corners[triangle_table_cube[3][2]]);
                            
                        glm::mat4 matrix = identity_matrix_4;
                        if(normal != core.z_dir) {
                            if(normal == -core.z_dir) {
                                matrix = rot_z_4;
                            } else {
                                glm::vec3 cross = glm::normalize(glm::cross(normal, core.z_dir));
                                float angle = glm::acos(dot(normal, core.z_dir));
                                matrix = glm::rotate(angle, cross);

                                matrix = glm::lookAt(glm::vec3(0, 0, 0), -normal, core.z_dir);
                            }
                        }

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + corners[triangle_table_cube[3][k * 3 + l]];
                                glm::vec2 tex_coord = (matrix * glm::vec4(position, 1.0)).xy();
                                position -= 0.5f;
                                vertices.push_back({position, normal, tex_coord, tile_coord[voxels[0]]});
                            }   
                        }
                    }
                    if(voxels[4] == 0) {
                        glm::vec3 normal = glm::cross(corners[triangle_table_cube[4][0]] - corners[triangle_table_cube[4][2]], corners[triangle_table_cube[4][1]] - corners[triangle_table_cube[4][2]]);
                            
                        glm::mat4 matrix = identity_matrix_4;
                        if(normal != core.z_dir) {
                            if(normal == -core.z_dir) {
                                matrix = rot_z_4;
                            } else {
                                glm::vec3 cross = glm::normalize(glm::cross(normal, core.z_dir));
                                float angle = glm::acos(dot(normal, core.z_dir));
                                matrix = glm::rotate(angle, cross);

                                matrix = glm::lookAt(glm::vec3(0, 0, 0), -normal, core.z_dir);
                            }
                        }

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + corners[triangle_table_cube[4][k * 3 + l]];
                                glm::vec2 tex_coord = (matrix * glm::vec4(position, 1.0)).xy();
                                position -= 0.5f;
                                vertices.push_back({position, normal, tex_coord, tile_coord[voxels[0]]});
                            }   
                        }
                    }
                    if(voxels[3] == 0) {
                        glm::vec3 normal = glm::cross(corners[triangle_table_cube[5][0]] - corners[triangle_table_cube[5][2]], corners[triangle_table_cube[5][1]] - corners[triangle_table_cube[5][2]]);
                            
                        glm::mat4 matrix = identity_matrix_4;
                        if(normal != core.z_dir) {
                            if(normal == -core.z_dir) {
                                matrix = rot_z_4;
                            } else {
                                glm::vec3 cross = glm::normalize(glm::cross(normal, core.z_dir));
                                float angle = glm::acos(dot(normal, core.z_dir));
                                matrix = glm::rotate(angle, cross);

                                matrix = glm::lookAt(glm::vec3(0, 0, 0), -normal, core.z_dir);
                            }
                        }

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + corners[triangle_table_cube[5][k * 3 + l]];
                                glm::vec2 tex_coord = (matrix * glm::vec4(position, 1.0)).xy();
                                position -= 0.5f;
                                vertices.push_back({position, normal, tex_coord, tile_coord[voxels[0]]});
                            }   
                        }
                    }
                }*/

                uint8_t i = 0;

                for(int a = 0; a < 8; ++a) {
                    i |= ((uint8_t(voxels[a]) != 0) << a);
                }
                
                if(i != 0 && i != 255) {
                    uint16_t tex_id;

                    int c = 0;
                    while(true) {
                        tex_id = voxels[c];
                        if(tex_id != 0) break;
                        ++c;
                    }
                    
                    for(int k = 0; k < triangle_table[i].size() / 3; ++k) {
                        glm::vec3 normal = normal_table[i][k];
                        glm::mat4 matrix = identity_matrix_4;
                        if(normal != core.z_dir) {
                            if(normal == -core.z_dir) {
                                matrix = rot_z_4;
                            } else {
                                glm::vec3 cross = glm::normalize(glm::cross(normal, core.z_dir));
                                float angle = glm::acos(dot(normal, core.z_dir));
                                matrix = glm::rotate(angle, cross);

                                matrix = glm::lookAt(glm::vec3(0, 0, 0), -normal, core.z_dir);
                            }
                        }

                        for(int l = 0; l < 3; ++l) {
                            glm::vec3 position = pos + midpoint_vertices[triangle_table[i][k * 3 + l]];
                            glm::vec2 tex_coord = (matrix * glm::vec4(position, 1.0)).xy();
                            vertices.push_back({position, normal, tex_coord, tile_coord[tex_id]});
                        }   
                    }
                }
            }
        }
    }

    if(status == 0) status = 1;
}

void Chunk::load_buffers() {
    if(!buffer.initialized) buffer.init();

    buffer.set_data(vertices.data(), vertices.size(), sizeof(vertices[0]));
    buffer.set_attrib(0, 3, sizeof(float) * 10, 0);
    buffer.set_attrib(1, 3, sizeof(float) * 10, sizeof(float) * 3);
    buffer.set_attrib(2, 2, sizeof(float) * 10, sizeof(float) * 6);
    buffer.set_attrib(3, 2, sizeof(float) * 10, sizeof(float) * 8);

    vertices.clear();

    status = 2;
}

uint16_t null_ref = 0;

uint16_t& get_block(glm::ivec3 pos) {
    glm::ivec3 key = glm::floor((glm::vec3)pos / 32.0f);
    if(core.chunks.contains(key)) {
        return (*core.chunks[key].data)[mod(pos.x, 32)][mod(pos.y, 32)][mod(pos.z, 32)];
    } else {
        return null_ref;
    }
}

bool get_block(glm::ivec3 pos, uint16_t& output) {
    glm::ivec3 key = glm::floor((glm::vec3)pos / 32.0f);
    if(core.chunks.contains(key)) {
        output = (*core.chunks[key].data)[mod(pos.x, 32)][mod(pos.y, 32)][mod(pos.z, 32)];
        return true;
    } else {
        return false;
    }
}

uint16_t& get_block(glm::ivec3 chunk_pos, glm::ivec3 block_pos) {
    if(core.chunks.contains(chunk_pos)) {
        return (*core.chunks[chunk_pos].data)[block_pos.x][block_pos.y][block_pos.z];
    } else {
        return null_ref;
    }
}

void block_update(glm::ivec3 pos) {
    glm::ivec3 i = glm::floor((glm::vec3)pos / 32.0f);
    glm::ivec3 block_i = mod(pos, 32);
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
                } else if(block_i.x == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    i += glm::ivec3{0, 0, -1};
                    core.changed_chunks.insert(i);
                    glm::ivec3 below_i = mod(below_coord, 32);
                    if(below_i.x == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                    } else if(below_i.x == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                    }
                    if(below_i.y == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                    } else if(below_i.y == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                    }
                } else if(block_i.z == 1) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 31) {
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
            } else if(north == 0 && (below_north == 0 || below_north == 3 || below_north == 5)) {
                focus_block = 0;
                north = 5;
                core.changed_chunks.insert(i);
                if(block_i.x == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 30) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                } else if(block_i.y == 31) {
                    i += glm::ivec3{0, 1, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 north_i = mod(north_coord, 32);
                    if(north_i.x == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                    } else if(north_i.x == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                    }
                    if(north_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(north_i.z == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
            } else if(east == 0 && (below_east == 0 || below_east == 3 || below_east == 5)) {
                focus_block = 0;
                east = 5;
                core.changed_chunks.insert(i);
                
                if(block_i.x == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 30) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                } else if(block_i.x == 31) {
                    i += glm::ivec3{1, 0, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 east_i = mod(east_coord, 32);
                    if(east_i.y == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                    } else if(east_i.y == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                    }
                    if(east_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(east_i.z == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
            } else if(south == 0 && (below_south == 0 || below_south == 3 || below_south == 5)) {
                focus_block = 0;
                south = 5;
                core.changed_chunks.insert(i);
                if(block_i.x == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    i += glm::ivec3{0, -1, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 south_i = mod(south_coord, 32);
                    if(south_i.x == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                    } else if(south_i.x == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                    }
                    if(south_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(south_i.z == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                } else if(block_i.y == 1) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                }

                core.block_updates.insert(pos + glm::ivec3{-1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{1, 0, 0});
                core.block_updates.insert(pos + glm::ivec3{0, -1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 1, 0});
                core.block_updates.insert(pos + glm::ivec3{0, 0, -1});
                core.block_updates.insert(pos + glm::ivec3{0, 0, 1});
            } else if(west == 0 && (below_west == 0 || below_west == 3 || below_west == 5)) {
                focus_block = 0;
                west = 5;
                core.changed_chunks.insert(i);

                if(block_i.x == 0) {
                    i += glm::ivec3{-1, 0, 0};
                    core.changed_chunks.insert(i);
                    glm::ivec3 west_i = mod(west_coord, 32);
                    if(west_i.y == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                    } else if(west_i.y == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                    }
                    if(west_i.z == 0) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                    } else if(west_i.z == 31) {
                        core.changed_chunks.insert(i + glm::ivec3{0, 0, 1});
                    }
                } else if(block_i.x == 1) {
                    core.changed_chunks.insert(i + glm::ivec3{-1, 0, 0});
                } else if(block_i.x == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{1, 0, 0});
                }
                if(block_i.y == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, -1, 0});
                } else if(block_i.y == 31) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 1, 0});
                }
                if(block_i.z == 0) {
                    core.changed_chunks.insert(i + glm::ivec3{0, 0, -1});
                } else if(block_i.z == 31) {
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
        if(t_collective > limit) {
            return false;
        }
        
        uint16_t block;
        bool status = get_block(current_voxel, block);
        if(status == false) return false;

        if(block != 0) {
            output = current_voxel;
            return true;
        }

        float t_x = (float(current_voxel.x + !(step_x >> 31)) - pos.x) / dir_norm.x;
        float t_y = (float(current_voxel.y + !(step_y >> 31)) - pos.y) / dir_norm.y;
        float t_z = (float(current_voxel.z + !(step_z >> 31)) - pos.z) / dir_norm.z;

        if(t_x < t_y && t_x < t_z) {
            pos += dir_norm * t_x;
            t_collective += t_x;
            current_voxel.x += step_x;
        } else if(t_y < t_x && t_y < t_z) {
            pos += dir_norm * t_y;
            t_collective += t_y;
            current_voxel.y += step_y;
        } else {
            pos += dir_norm * t_z;
            t_collective += t_z;
            current_voxel.z += step_z;
        }
    }
}

bool raycast_place(glm::vec3 pos, glm::vec3 dir, glm::ivec3& output, float limit) {
    glm::vec3 dir_norm = glm::normalize(dir);

    glm::ivec3 current_voxel = glm::floor(pos);
    int step_x = (dir.x < 0) ? -1 : 1;
    int step_y = (dir.y < 0) ? -1 : 1;
    int step_z = (dir.z < 0) ? -1 : 1;

    float t_collective = 0.0;

    while(true) {
        float t_x = (float(current_voxel.x + !(step_x >> 31)) - pos.x) / dir_norm.x;
        float t_y = (float(current_voxel.y + !(step_y >> 31)) - pos.y) / dir_norm.y;
        float t_z = (float(current_voxel.z + !(step_z >> 31)) - pos.z) / dir_norm.z;

        if(t_x < t_y && t_x < t_z) {
            pos += dir_norm * t_x;
            t_collective += t_x;
            current_voxel.x += step_x;
            
            if(t_collective > limit) {
                return false;
            }

            uint16_t& block = get_block(current_voxel);
            if(block != 0) {
                current_voxel.x -= step_x;
                output = current_voxel;
                return true;
            }
        } else if(t_y < t_x && t_y < t_z) {
            pos += dir_norm * t_y;
            t_collective += t_y;
            current_voxel.y += step_y;

            if(t_collective > limit) {
                return false;
            }

            uint16_t& block = get_block(current_voxel);
            if(block != 0) {
                current_voxel.y -= step_y;
                output = current_voxel;
                return true;
            }
        } else {
            pos += dir_norm * t_z;
            t_collective += t_z;
            current_voxel.z += step_z;

            if(t_collective > limit) {
                return false;
            }

            uint16_t& block = get_block(current_voxel);
            if(block != 0) {
                current_voxel.z -= step_z;
                output = current_voxel;
                return true;
            }
        }
    }
}

#include "gui\parameter_functions.cpp"

void init_widgets(Gui_core& self) {
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x1E}});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x3C}});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x5A}});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, core.viewport_size / 2 + glm::ivec2{-5, -9}, self.font, "+"});
    self.widgets.emplace_back(Text{2, {1.0, 1.0, 1.0, 1.0}, {0xC, -0x78}});
}

void chunk_load_func() {
    while(core.game_running) {
        if(core.chunk_gen_queue.size() != 0) {
            core.gen_queue_mutex.lock();
            glm::ivec3 key = core.chunk_gen_queue.top();
            core.chunk_gen_queue.pop();
            core.gen_queue_mutex.unlock();


            if(!core.chunks.contains(key)) {
                core.chunk_allocate_thread_mutex.lock();
                core.chunks.insert({key, Chunk(key)});

                core.chunks[key].generate();
                core.chunk_allocate_thread_mutex.unlock();
            }

            if(core.chunks[key].status == 0 || core.chunks[key].status == 3) {
                core.chunks[key].create_mesh();

                core.buffer_update_mutex.lock();
                core.chunk_buffer_updates.push_back(key);
                core.buffer_update_mutex.unlock();
            }
        }
    }
}

void Core::init() {
    screen_matrix = glm::translate(glm::inverse(glm::scale(identity_matrix_3, glm::vec2(viewport_size / 2))), glm::vec2(-viewport_size / 2));
    gui_framebuffer.init(viewport_size.x, viewport_size.y);
    space_core.framebuffer.init(viewport_size.x, viewport_size.y);

    grid_shader.compile("res/shaders/grid_3d.vs", "res/shaders/grid_3d_xy.fs");
    chunk_shader.compile("res/shaders/chunk_3d.vs", "res/shaders/chunk_3d.fs");
    screen_shader.compile("res/shaders/screen.vs", "res/shaders/screen.fs");
    screen_shader_solid.compile("res/shaders/screen_solid.vs", "res/shaders/screen_solid.fs");
    cube_shader.compile("res/shaders/cube.vs", "res/shaders/cube.fs");

    gui_core.init();

    fnfractal->SetSource(fnperlin);
    fnfractal->SetOctaveCount(6);

    tex.load("res/tilex.png", true);

    space_core.init();

    chunk_thread = std::thread(
        []() {
            chunk_load_func();
        }
    );
}