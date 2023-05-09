#pragma once
#include "stuff.cpp"
#include "gui\gui.cpp"
#include "marching_cubes.cpp"

template<typename type>
std::string to_hex(type num) {
    bool neg = num < 0;

    std::string ret;

    if(neg) {
        ret += '-';
        num *= -1;
    }

    bool zeroes = false;

    for(int i = sizeof(i) - 1; i >= 0; --i) {
        uint8_t byte = (num >> (i * 8)) & 0xFF;

        uint8_t a = byte >> 4;
        uint8_t b = byte & 0xF;

        if(a != 0 || zeroes) {
            zeroes = true;
            if(a < 0xA) {
                a = '0' + a;
            } else {
                a = 0x76 + a;
            }
            ret += a;
        }

        if(b != 0 || zeroes) {
            zeroes = true;
            if(b < 0xA) {
                b = '0' + b;
            } else {
                b = 0x76 + b;
            }
            ret += b;
        }
    }

    if(!zeroes) ret = '0';

    return ret;
}

template<typename type>
std::string to_quat(type num) {
    bool neg = num < 0;

    std::string ret;

    if(neg) {
        ret += '-';
        num *= -1;
    }

    bool zeroes = false;

    for(int i = sizeof(i) - 1; i >= 0; --i) {
        uint8_t byte = (num >> (i * 8)) & 0xFF;

        uint8_t a = byte >> 6;
        uint8_t b = (byte >> 4) & 3;
        uint8_t c = (byte >> 2) & 3;
        uint8_t d = byte & 3;

        if(a != 0 || zeroes) {
            zeroes = true;
            if(a < 2) {
                a = '0' + a;
            } else {
                a = 0x84 + a;
            }
            ret += a;
        }

        if(b != 0 || zeroes) {
            zeroes = true;
            if(b < 2) {
                b = '0' + b;
            } else {
                b = 0x84 + b;
            }
            ret += b;
        }

        if(c != 0 || zeroes) {
            zeroes = true;
            if(c < 2) {
                c = '0' + c;
            } else {
                c = 0x84 + c;
            }
            ret += c;
        }

        if(d != 0 || zeroes) {
            zeroes = true;
            if(d < 2) {
                d = '0' + d;
            } else {
                d = 0x84 + d;
            }
            ret += d;
        }
    }

    if(!zeroes) ret = '0';

    return ret;
}

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

glm::vec2 rand_vec() {
    float random = (float(rand()) / 32768) * M_PI * 2;

    return {cos(random), sin(random)};
}

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

                for(Planet& p : planets) {
                    p.velocity += p.acceleration * delta_time * sim_speed;
                    p.acceleration = {0, 0, 0};
                    p.position += p.velocity * delta_time * sim_speed;
                }
            }
        }
    }

    void orbit(Planet& a, Planet& b) {
        a.position += b.position;
        glm::dvec3 dist = b.position - a.position;
        glm::dvec3 dir = normalize(cross(dist, {0, 0, 1}));
        a.velocity = dir * sqrt(G * b.mass / glm::length(dist)) + b.velocity;
    }

    void init() {
        for(int i = 0; i < 6; ++i) {
            textures.push_back(Texture());
        }
        textures[0].load("res/planet_surfaces/gas_giant.png");
        textures[1].load("res/planet_surfaces/ice_giant.png");
        textures[2].load("res/planet_surfaces/forest.png");
        textures[3].load("res/planet_surfaces/desert.png");
        textures[4].load("res/planet_surfaces/icy.png");
        textures[5].load("res/planet_surfaces/dusty.png");
        textures[6].load("res/planet_surfaces/barren.png");

        planet_shader.compile("res/shaders/planet.vs", "res/shaders/planet.fs");
        planet_shader_solid.compile("res/shaders/planet_solid.vs", "res/shaders/planet_solid.fs");

        planets.push_back(Planet(glm::vec3{0, 0, 0}, 0x20, 0x1000, 87, -1, true, {1.0, 0.8, 0.5}, true));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0xE0, 0x5.0p0f, 0xF.0p0f, 0x80, 1));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x200, 0x1.0p0f, 0x1.0p0f, 0x80, 2));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x7, 0x0.5p0f, 0x0.2p0f, 0x80, 6));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x300, 0x0.Cp0f, 0x0.8p0f, 0x80, 5));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x460, 0x1.Cp0f, 0x2.8p0f, 0x80, 4));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x640, 0x6.8p0f, 0x1C.0p0f, 0x80, 0));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x14, 0x0.8p0f, 0x0.4p0f, 0x80, 3));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x24, 0x0.6p0f, 0x0.28p0f, 0x80, 4));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x38, 0x0.38p0f, 0x0.1p0f, 0x80, 4));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x6C0, 0x3.8p0f, 0xC.0p0f, 0x80, 1));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0xC, 0x0.6p0f, 0x0.2p0f, 0x80, 4));
        planets.push_back(Planet(glm::normalize(glm::vec3{rand_vec(), 0.0}) * (float)0x28, 0x0.28p0f, 0x0.08p0f, 0x80, 4));

        orbit(planets[1], planets[0]);
        orbit(planets[2], planets[0]);
        orbit(planets[3], planets[2]);
        orbit(planets[4], planets[0]);
        orbit(planets[5], planets[0]);
        orbit(planets[6], planets[0]);
        orbit(planets[7], planets[6]);
        orbit(planets[8], planets[6]);
        orbit(planets[9], planets[6]);
        orbit(planets[10], planets[0]);
        orbit(planets[11], planets[10]);
        orbit(planets[12], planets[10]);

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

const glm::ivec3 region = {5, 5, 5};

const int factor = 32;
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

struct Chunk {
    glm::ivec3 index;
    std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x20>, 0x20>, 0x20>> voxels = std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x20>, 0x20>, 0x20>>(new std::array<std::array<std::array<uint16_t, 0x20>, 0x20>, 0x20>);
    
    std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x10>, 0x10>, 0x10>> terrain_voxels = std::unique_ptr<std::array<std::array<std::array<uint16_t, 0x10>, 0x10>, 0x10>>(new std::array<std::array<std::array<uint16_t, 0x10>, 0x10>, 0x10>);
    std::unique_ptr<std::array<std::array<std::array<glm::vec3, 0x11>, 0x11>, 0x11>> surface_net = std::unique_ptr<std::array<std::array<std::array<glm::vec3, 0x11>, 0x11>, 0x11>>(new std::array<std::array<std::array<glm::vec3, 0x11>, 0x11>, 0x11>);
    
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

enum dir:uint8_t{NUL, NORTH, EAST, SOUTH, WEST, UP, DOWN};

struct Core {
    bool game_running = true;
    GLFWwindow* window;
    glm::ivec2 screen_size = {800, 600};
    
    glm::ivec2 viewport_size = {800, 600};
    //glm::ivec2 monitor_size = {0, 0};
    glm::ivec2 cursor_pos = {0, 0};
    bool cursor_hidden = false;

    uint16_t active_voxel = 7;

    glm::mat3 screen_matrix;

    glm::vec3 view_pos = {-9, 0, 5};
    glm::vec3 view_dir = {0, 1, 0};
    glm::vec3 up_dir = {0, 0, 1};
    glm::vec3 z_dir = {0, 0, 1};
    double move_speed = 16;
    uint8_t dir_enum = NUL;

    float gravity = -20;
    glm::vec3 accel = {0, 0, gravity};
    glm::vec3 vel = {0, 0, 0};

    bool chunk_debug = false;

    Shader grid_shader;
    Shader chunk_shader;
    Shader screen_shader;
    Shader screen_shader_solid;
    Shader cube_shader;
    Shader billboard_shader;
    Shader chunk_debug_shader;
    Shader any_shader;

    Texture tex;
    Texture x_tex;

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
    FastNoise::SmartNode<FastNoise::FractalFBm> fnbiome = FastNoise::New<FastNoise::FractalFBm>();


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
            glm::mat4 rotate_matrix_x = glm::rotate(identity_matrix_4, float(2 * M_PI * 0x0.004p0 * -difference.x), up_dir);
            glm::mat4 rotate_matrix_y = glm::rotate(identity_matrix_4, float(2 * M_PI * 0x0.004p0 * difference.y), glm::cross(view_dir, up_dir));
            glm::vec3 new_view_dir = (rotate_matrix_y * glm::vec4(view_dir, 1.0));

            if(new_view_dir.z < 0x0.FFp0f && new_view_dir.z > -0x0.FFp0f) view_dir = rotate_matrix_x * glm::vec4(new_view_dir, 1.0);
            else view_dir = rotate_matrix_x * glm::vec4(view_dir, 1.0);
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

struct voxel_info {
    std::vector<glm::vec2> texture;
    std::array<uint8_t, 6> pt = {0, 0, 0, 0, 0, 0};
    bool push = false;
    bool spread = false;
    bool block = false;

    voxel_info() = default;

    voxel_info(glm::vec2 tex, bool b = false) {
        texture.push_back(tex);
        block = b;
    }

    voxel_info(std::vector<glm::vec2>&& tex_vec, std::array<uint8_t, 6>&& ptr, bool b = false, bool p = false, bool s = false) {
        texture = std::move(tex_vec);
        pt = std::move(ptr);
        push = p;
        spread = s;
        block = b;
    }

    voxel_info(voxel_info&& v) = default;

    voxel_info(const voxel_info& v) = default;
};

std::unordered_map<uint16_t, voxel_info> voxel_data = {
    {0, voxel_info({0, 0}, true)},
    {1, voxel_info({1, 0}, true)},
    {2, voxel_info({{1, 0}, {2, 0}, {3, 0}}, {2, 2, 2, 2, 0, 1}, true, true, true)},
    {3, voxel_info({1, 1}, true)},
    {4, voxel_info({{1, 3}, {2, 3}, {3, 3}}, {2, 2, 2, 2, 0, 1}, true, true, true)},
    {5, voxel_info({1, 2}, true)},
    {6, voxel_info({0, 2}, true)},
    {7, voxel_info({{2, 2}, {3, 2}}, {0, 0, 0, 0, 1, 1}, true)},
    {8, voxel_info({2, 1}, true)},
    {9, voxel_info({3, 1}, true)}
};

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
    glm::vec3 chunk_vec = index * 0x20;
    std::array<std::array<std::array<float, 0x20>, 0x20>, 0x21> noise;
    core.fnfractal->GenUniformGrid3D(&noise[0][0][0], index.x * 0x20, index.y * 0x20, index.z * 0x20, 0x20, 0x20, 0x21, freq / 2, 0x1B);
    std::array<std::array<std::array<float, 0x20>, 0x20>, 0x21> heightmap;
    
    /*std::array<std::array<uint16_t, 0x20>, 0x20> biome;
    std::array<std::array<std::array<float, 0x20>, 0x20>, 0x20> humidity;
    std::array<std::array<std::array<float, 0x20>, 0x20>, 0x20> elev;
    core.fnfractal->GenUniformGrid3D(&humidity[0][0][0], index.x * 0x20 + 1823, index.y * 0x20 + 2638, index.z * 0x20 + 2738, 0x20, 0x20, 0x20, freq / 2, 0x1B);
    core.fnfractal->GenUniformGrid3D(&elev[0][0][0], index.x * 0x20 - 82, index.y * 0x20 - 2532, index.z * 0x20 - 275, 0x20, 0x20, 0x20, freq / 2, 0x1B);
    */
    for(float x = 0; x < 0x20; ++x) {
        for(float y = 0; y < 0x20; ++y) {
            for(float z = 0; z < 0x21; ++z) {
                heightmap[z][y][x] = noise[z][y][x];
                heightmap[z][y][x] = heightmap[z][y][x];
            }
        }
    }

    auto& terrain_voxels_ref = (*terrain_voxels.get());
    auto& voxels_ref = (*voxels.get());

    for(float x = 0; x < 0x10; ++x) {
        for(float y = 0; y < 0x10; ++y) {
            for(float z = 0; z < 0x10; ++z) {
                terrain_voxels_ref[x][y][z] = 0;
            }
        }
    }

    for(float x = 0; x < 0x20; ++x) {
        for(float y = 0; y < 0x20; ++y) {
            for(float z = 0; z < 0x20; ++z) {
                float density = heightmap[z][y][x];

                uint16_t vox = 0;

                if(density > 0.0) {
                    if(density < 0.10) {
                        if(heightmap[z + 1][y][x] <= 0.0) {
                            vox = 2;
                        } else {
                            vox = 1;
                        }
                    } else {
                        vox = 3;
                    }
                }
                
                /*switch(biome[y][x]) {
                    case 1: { // mountain
                        if(density > 0.0) {
                            vox = 3;
                        }
                        break;
                    }
                    case 2: { // virpin
                        if(density > 0.0) {
                            if(density < 0.15 * 48) {
                                if(heightmap[z + 1][y][x] <= 0.0) {
                                    vox = 4;
                                } else {
                                    vox = 1;
                                }
                            } else {
                                vox = 3;
                            }
                        }
                        break;
                    }
                    case 3: { // grassland
                        if(density > 0.0) {
                            if(density < 0.25 * 48) {
                                if(heightmap[z + 1][y][x] <= 0.0) {
                                    vox = 2;
                                } else {
                                    vox = 1;
                                }
                            } else {
                                vox = 3;
                            }
                        }
                        break;
                    }
                    case 4: { // desert
                        if(density > 0.0) {
                            if(density < 0.15 * 48) {
                                vox = 5;
                            } else if(density < 0.3 * 48) {
                                vox = 6;
                            } else {
                                vox = 3;
                            }
                        }
                        break;
                    }
                }*/
                //if(vox != 0 && chunk_vec.z + z > 32) vox = 3;
                //if(vox == 0 && 256 - m > 0.0f) vox = 4;
                voxels_ref[x][y][z] = vox;
            }
        }
    }

    /*for(int x = 0; x < 32; ++x) {
        for(int y = 0; y < 32; ++y) {
            for(int z = 0; z < 32; ++z) {
                uint16_t vox = 0;//((rand() >= 32766) ? 9 : 0);

                voxels_ref[x][y][z] = vox;
            }
        }
    }*/
    // std::max(std::max(abs(x + chunk_vec.x), abs(y + chunk_vec.y)), abs(z + chunk_vec.z))
    // result[x + y * 0x20 + z * 0x400] * 6 + 0x80 - glm::length(glm::vec3(x, y, z) + glm::vec3(chunk_vec)) > 0.0f
}

std::array<uint16_t, 8> get_surrounding_terrain_voxels(int x, int y, int z, std::array<std::array<std::array<std::array<uint16_t, 16>, 16>, 16>*, 8> arr) {
    std::array<uint16_t, 8> voxels;

    for(uint8_t ix = 0; ix < 2; ix++) {
        for(uint8_t iy = 0; iy < 2; iy++) {
            for(uint8_t iz = 0; iz < 2; iz++) {
                glm::ivec3 v = {x + ix, y + iy, z + iz};
                int i = ix + iy * 2 + iz * 4;

                glm::ivec3 chunk_index = {v.x > 0x1F, v.y > 0x1F, v.z > 0x1F};
                glm::ivec3 voxel_index = v - (chunk_index * 0x20);

                voxels[i] = (*arr[chunk_index.x + chunk_index.y * 2 + chunk_index.z * 4])[voxel_index.x][voxel_index.y][voxel_index.z];
            }
        }
    }

    return voxels;
}

std::array<uint16_t, 8> get_surrounding_terrain_voxels(int x, int y, int z, std::array<std::array<std::array<std::array<uint16_t, 16>, 16>, 16>*, 27> arr) {
    std::array<uint16_t, 8> voxels;

    for(uint8_t ix = 0; ix < 2; ix++) {
        for(uint8_t iy = 0; iy < 2; iy++) {
            for(uint8_t iz = 0; iz < 2; iz++) {
                glm::ivec3 v = {x + ix, y + iy, z + iz};
                int i = ix + iy * 2 + iz * 4;

                glm::ivec3 chunk_index = {floor(float(v.x) / 0x10), floor(float(v.y) / 0x10), floor(float(v.z) / 0x10)};
                glm::ivec3 voxel_index = v - (chunk_index * 0x10);

                voxels[i] = (*arr[(chunk_index.x + 1) + (chunk_index.y + 1) * 3 + (chunk_index.z + 1) * 9])[voxel_index.x][voxel_index.y][voxel_index.z];
            }
        }
    }

    return voxels;
}

std::array<uint16_t, 27> get_surrounding_terrain_voxels(glm::ivec3 pos, std::array<std::array<std::array<std::array<uint16_t, 16>, 16>, 16>*, 27> arr) {
    std::array<uint16_t, 27> voxels;

    for(uint8_t ix = 0; ix < 3; ix++) {
        for(uint8_t iy = 0; iy < 3; iy++) {
            for(uint8_t iz = 0; iz < 3; iz++) {
                glm::ivec3 v = {pos.x + ix - 1, pos.y + iy - 1, pos.z + iz - 1};
                int i = ix + iy * 3 + iz * 9;

                glm::ivec3 chunk_index = {floor(float(v.x) / 0x10), floor(float(v.y) / 0x10), floor(float(v.z) / 0x10)};
                glm::ivec3 voxel_index = v - (chunk_index * 0x10);

                voxels[i] = (*arr[(chunk_index.x + 1) + (chunk_index.y + 1) * 3 + (chunk_index.z + 1) * 9])[voxel_index.x][voxel_index.y][voxel_index.z];
            }
        }
    }

    return voxels;
}

std::array<uint16_t, 27> get_surrounding_voxels(glm::ivec3 pos, std::array<std::array<std::array<std::array<uint16_t, 32>, 32>, 32>*, 27> arr) {
    std::array<uint16_t, 27> voxels;

    for(uint8_t ix = 0; ix < 3; ix++) {
        for(uint8_t iy = 0; iy < 3; iy++) {
            for(uint8_t iz = 0; iz < 3; iz++) {
                glm::ivec3 v = {pos.x + ix - 1, pos.y + iy - 1, pos.z + iz - 1};
                int i = ix + iy * 3 + iz * 9;

                glm::ivec3 chunk_index = {floor(float(v.x) / 0x20), floor(float(v.y) / 0x20), floor(float(v.z) / 0x20)};
                glm::ivec3 voxel_index = v - (chunk_index * 0x20);

                voxels[i] = (*arr[(chunk_index.x + 1) + (chunk_index.y + 1) * 3 + (chunk_index.z + 1) * 9])[voxel_index.x][voxel_index.y][voxel_index.z];
            }
        }
    }

    return voxels;
}

glm::vec3 get_point(std::array<uint16_t, 8> voxels) {
    //return {0.5, 0.5, 0.5};
    
    std::vector<glm::vec3> vec;

    uint8_t i = 0;

    for(int a = 0; a < 8; ++a) i |= (!(voxel_data[voxels[a]].block == true) << a);

    if(i == 0 || i == 255) return {0.5, 0.5, 0.5};

    std::set<uint8_t> s;

    for(uint8_t a : triangle_table[i]) s.insert(a);

    for(uint8_t a : s) vec.push_back(midpoint_vertices[a]);

    glm::vec3 avg = {0, 0, 0};
    for(glm::vec3 v : vec) avg += v;

    avg /= float(vec.size());

    return avg;
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

    for(int x = -1; x < 2; ++x) {
        for(int y = -1; y < 2; ++y) {
            for(int z = -1; z < 2; ++z) {
                if(!core.chunks.contains(index + glm::ivec3{x, y, z})) {
                    keys.push_back(index + glm::ivec3{x, y, z});
                }
            }
        }
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

/*glm::vec3 get_point(std::array<uint16_t, 8> voxels) {
    std::vector<glm::vec3> vec;

    uint8_t i = 0;

    for(int a = 0; a < 8; ++a) i |= (!(voxels[a] == 0) << a);

    if(i == 0 || i == 255) return {0.5, 0.5, 0.5};

    std::set<uint8_t> s;

    for(uint8_t a : triangle_table[i]) s.insert(a);

    for(uint8_t a : s) vec.push_back(midpoint_vertices[a]);

    glm::vec3 avg = {0, 0, 0};
    for(glm::vec3 v : vec) avg += v;

    avg /= float(vec.size());

    return avg;
}*/



void Chunk::create_mesh() {
    load_neighbors();

    std::array<std::array<std::array<std::array<uint16_t, 16>, 16>, 16>*, 27> arr27 = {};

    for(int x = -1; x < 2; ++x) {
        for(int y = -1; y < 2; ++y) {
            for(int z = -1; z < 2; ++z) {
                arr27[(x + 1) + (y + 1) * 3 + (z + 1) * 9] = &(*core.chunks[index + glm::ivec3{x, y, z}].terrain_voxels.get());
            }
        }
    }

    auto& sn = *surface_net.get();

    std::array<std::array<std::array<std::array<uint16_t, 16>, 16>, 16>*, 8> arr = {arr27[13], arr27[14], arr27[16], arr27[17], arr27[22], arr27[23], arr27[25], arr27[26]};

    for(int8_t x = -1; x < 0x10; ++x) {
        for(int8_t y = -1; y < 0x10; ++y) {
            for(int8_t z = -1; z < 0x10; ++z) {
                sn[x + 1][y + 1][z + 1] = (glm::vec3(x, y, z) + get_point(get_surrounding_terrain_voxels(x, y, z, arr27))) + 0.5f;
            }
        }
    }

    for(uint8_t x = 0; x < 0x10; ++x) {
        for(uint8_t y = 0; y < 0x10; ++y) {
            for(uint8_t z = 0; z < 0x10; ++z) {
                glm::vec3 pos = {x, y, z};

                std::array<uint16_t, 27> voxels = get_surrounding_terrain_voxels({x, y, z}, arr27);
                
                if(voxels[13] == 0) {
                    if(voxels[14] != 0) {
                        voxel_info& v = voxel_data[voxels[14]];

                        if(v.spread && voxel_data[voxels[4]].spread) {
                            for(int k = 0; k < 2; ++k) {
                                glm::vec3 a = corners[triangle_table_cube[0][k * 3 + 0]];
                                glm::vec3 b = corners[triangle_table_cube[0][k * 3 + 1]];
                                glm::vec3 c = corners[triangle_table_cube[0][k * 3 + 2]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + a, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + b, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x + 1][y + a.y][z + a.z], sn[x + b.x + 1][y + b.y][z + b.z], sn[x + c.x + 1][y + c.y][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                if(isnan(x_vec.x)) {
                                    if(normal.z > 0) {
                                        x_vec = {1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    } else {
                                        x_vec = {-1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    }
                                }
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }   
                            }
                        } else {
                            if(v.push) {
                                glm::vec3 a = corners[triangle_table_cube[0][3]];
                                glm::vec3 b = corners[triangle_table_cube[0][4]];
                                glm::vec3 c = corners[triangle_table_cube[0][5]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + a, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + b, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x + 1][y + a.y][z + a.z], sn[x + b.x + 1][y + b.y][z + b.z], sn[x + c.x + 1][y + c.y][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(ver[1] - ver[2]);
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));

                                float dot_top_corner = dot(y_vec, ver[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[0]]});
                                }

                                a = corners[triangle_table_cube[0][0]];
                                b = corners[triangle_table_cube[0][1]];
                                c = corners[triangle_table_cube[0][2]];
                                std::array<glm::vec3, 3> ver2;
                                if(v.block) ver2 = {glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + a, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + b, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + c};
                                else ver2 = {sn[x + a.x + 1][y + a.y][z + a.z], sn[x + b.x + 1][y + b.y][z + b.z], sn[x + c.x + 1][y + c.y][z + c.z]};
                                normal = glm::normalize(glm::cross(ver2[1] - ver2[0], ver2[2] - ver2[0]));

                                

                                x_vec = glm::normalize(x_vec - normal * dot(normal, x_vec));
                                y_vec = glm::normalize(glm::cross(normal, x_vec));

                                dot_top_corner = dot(y_vec, ver2[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver2[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[0]]});
                                }
                            } else {
                                for(int k = 0; k < 2; ++k) {
                                    glm::vec3 a = corners[triangle_table_cube[0][k * 3 + 0]];
                                    glm::vec3 b = corners[triangle_table_cube[0][k * 3 + 1]];
                                    glm::vec3 c = corners[triangle_table_cube[0][k * 3 + 2]];
                                    std::array<glm::vec3, 3> ver;
                                    if(v.block) ver = {glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + a, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + b, glm::vec3(1, 0, 0) + glm::vec3(x, y, z) + c};
                                    else ver = {sn[x + a.x + 1][y + a.y][z + a.z], sn[x + b.x + 1][y + b.y][z + b.z], sn[x + c.x + 1][y + c.y][z + c.z]};
                                    glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                    
                                    glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                    glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                    if(isnan(x_vec.x)) {
                                        if(normal.z > 0) {
                                            x_vec = {1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        } else {
                                            x_vec = {-1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        }
                                    }
                                    
                                    for(int l = 0; l < 3; ++l) {
                                        glm::vec3 position = ver[l];
                                        glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                        vertices.push_back({position, normal, tex_coord, v.texture[v.pt[0]]});
                                    }   
                                }
                            }
                        }
                    }
                    if(voxels[16] != 0) {
                        voxel_info& v = voxel_data[voxels[16]];

                        if(v.spread && voxel_data[voxels[4]].spread) {
                            for(int k = 0; k < 2; ++k) {
                                glm::vec3 a = corners[triangle_table_cube[1][k * 3 + 0]];
                                glm::vec3 b = corners[triangle_table_cube[1][k * 3 + 1]];
                                glm::vec3 c = corners[triangle_table_cube[1][k * 3 + 2]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + a, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + b, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x][y + a.y + 1][z + a.z], sn[x + b.x][y + b.y + 1][z + b.z], sn[x + c.x][y + c.y + 1][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                if(isnan(x_vec.x)) {
                                    if(normal.z > 0) {
                                        x_vec = {1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    } else {
                                        x_vec = {-1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    }
                                }
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }   
                            }
                        } else {
                            if(v.push) {
                                glm::vec3 a = corners[triangle_table_cube[1][3]];
                                glm::vec3 b = corners[triangle_table_cube[1][4]];
                                glm::vec3 c = corners[triangle_table_cube[1][5]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + a, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + b, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x][y + a.y + 1][z + a.z], sn[x + b.x][y + b.y + 1][z + b.z], sn[x + c.x][y + c.y + 1][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(ver[1] - ver[2]);
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));

                                float dot_top_corner = dot(y_vec, ver[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[2]]});
                                }

                                a = corners[triangle_table_cube[1][0]];
                                b = corners[triangle_table_cube[1][1]];
                                c = corners[triangle_table_cube[1][2]];
                                std::array<glm::vec3, 3> ver2;
                                if(v.block) ver2 = {glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + a, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + b, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + c};
                                else ver2 = {sn[x + a.x][y + a.y + 1][z + a.z], sn[x + b.x][y + b.y + 1][z + b.z], sn[x + c.x][y + c.y + 1][z + c.z]};
                                normal = glm::normalize(glm::cross(ver2[1] - ver2[0], ver2[2] - ver2[0]));

                                

                                x_vec = glm::normalize(x_vec - normal * dot(normal, x_vec));
                                y_vec = glm::normalize(glm::cross(normal, x_vec));

                                dot_top_corner = dot(y_vec, ver2[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver2[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[2]]});
                                }
                            } else {
                                for(int k = 0; k < 2; ++k) {
                                    glm::vec3 a = corners[triangle_table_cube[1][k * 3 + 0]];
                                    glm::vec3 b = corners[triangle_table_cube[1][k * 3 + 1]];
                                    glm::vec3 c = corners[triangle_table_cube[1][k * 3 + 2]];
                                    std::array<glm::vec3, 3> ver;
                                    if(v.block) ver = {glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + a, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + b, glm::vec3(0, 1, 0) + glm::vec3(x, y, z) + c};
                                    else ver = {sn[x + a.x][y + a.y + 1][z + a.z], sn[x + b.x][y + b.y + 1][z + b.z], sn[x + c.x][y + c.y + 1][z + c.z]};
                                    glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                    
                                    glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                    glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                    if(isnan(x_vec.x)) {
                                        if(normal.z > 0) {
                                            x_vec = {1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        } else {
                                            x_vec = {-1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        }
                                    }
                                    
                                    for(int l = 0; l < 3; ++l) {
                                        glm::vec3 position = ver[l];
                                        glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                        vertices.push_back({position, normal, tex_coord, v.texture[v.pt[2]]});
                                    }   
                                }
                            }
                        }
                    }
                    if(voxels[22] != 0) {
                        voxel_info& v = voxel_data[voxels[22]];
                        
                        for(int k = 0; k < triangle_table_cube[2].size() / 3; ++k) {
                            glm::vec3 a = corners[triangle_table_cube[2][k * 3 + 0]];
                            glm::vec3 b = corners[triangle_table_cube[2][k * 3 + 1]];
                            glm::vec3 c = corners[triangle_table_cube[2][k * 3 + 2]];
                            std::array<glm::vec3, 3> ver;
                            if(v.block) ver = {glm::vec3(0, 0, 1) + glm::vec3(x, y, z) + a, glm::vec3(0, 0, 1) + glm::vec3(x, y, z) + b, glm::vec3(0, 0, 1) + glm::vec3(x, y, z) + c};
                            else ver = {sn[x + a.x][y + a.y][z + a.z + 1], sn[x + b.x][y + b.y][z + b.z + 1], sn[x + c.x][y + c.y][z + c.z + 1]};
                            glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                            
                            glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                            glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                            if(isnan(x_vec.x)) {
                                if(normal.z > 0) {
                                    x_vec = {1, 0, 0};
                                    y_vec = {0, 1, 0};
                                } else {
                                    x_vec = {-1, 0, 0};
                                    y_vec = {0, 1, 0};
                                }
                            }
                            
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = ver[l];
                                glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                vertices.push_back({position, normal, tex_coord, v.texture[v.pt[4]]});
                            }   
                        }
                    }
                } else {
                    voxel_info& v = voxel_data[voxels[13]];

                    if(voxels[14] == 0) {
                        if(v.spread && voxel_data[voxels[5]].spread) {
                            for(int k = 0; k < 2; ++k) {
                                glm::vec3 a = corners[triangle_table_cube[3][k * 3 + 0]];
                                glm::vec3 b = corners[triangle_table_cube[3][k * 3 + 1]];
                                glm::vec3 c = corners[triangle_table_cube[3][k * 3 + 2]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                if(isnan(x_vec.x)) {
                                    if(normal.z > 0) {
                                        x_vec = {1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    } else {
                                        x_vec = {-1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    }
                                }
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }
                            }
                        } else {
                            if(v.push) {
                                glm::vec3 a = corners[triangle_table_cube[3][3]];
                                glm::vec3 b = corners[triangle_table_cube[3][4]];
                                glm::vec3 c = corners[triangle_table_cube[3][5]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(ver[1] - ver[2]);
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));

                                float dot_top_corner = dot(y_vec, ver[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[1]]});
                                }

                                a = corners[triangle_table_cube[3][0]];
                                b = corners[triangle_table_cube[3][1]];
                                c = corners[triangle_table_cube[3][2]];
                                std::array<glm::vec3, 3> ver2;
                                if(v.block) ver2 = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                                else ver2 = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                normal = glm::normalize(glm::cross(ver2[1] - ver2[0], ver2[2] - ver2[0]));

                                

                                x_vec = glm::normalize(x_vec - normal * dot(normal, x_vec));
                                y_vec = glm::normalize(glm::cross(normal, x_vec));

                                dot_top_corner = dot(y_vec, ver2[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver2[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[1]]});
                                }
                            } else {
                                for(int k = 0; k < 2; ++k) {
                                    glm::vec3 a = corners[triangle_table_cube[3][k * 3 + 0]];
                                    glm::vec3 b = corners[triangle_table_cube[3][k * 3 + 1]];
                                    glm::vec3 c = corners[triangle_table_cube[3][k * 3 + 2]];
                                    std::array<glm::vec3, 3> ver;
                                    if(v.block) ver = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                                    else ver = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                    glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                    
                                    glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                    glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                    if(isnan(x_vec.x)) {
                                        if(normal.z > 0) {
                                            x_vec = {1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        } else {
                                            x_vec = {-1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        }
                                    }
                                    
                                    for(int l = 0; l < 3; ++l) {
                                        glm::vec3 position = ver[l];
                                        glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                        vertices.push_back({position, normal, tex_coord, v.texture[v.pt[1]]});
                                    }
                                }
                            }
                        }
                    }
                    if(voxels[16] == 0) {
                        if(v.spread && voxel_data[voxels[7]].spread) {
                            for(int k = 0; k < 2; ++k) {
                                glm::vec3 a = corners[triangle_table_cube[4][k * 3 + 0]];
                                glm::vec3 b = corners[triangle_table_cube[4][k * 3 + 1]];
                                glm::vec3 c = corners[triangle_table_cube[4][k * 3 + 2]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                if(isnan(x_vec.x)) {
                                    if(normal.z > 0) {
                                        x_vec = {1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    } else {
                                        x_vec = {-1, 0, 0};
                                        y_vec = {0, 1, 0};
                                    }
                                }
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }
                            }
                        } else {
                            if(v.push) {
                                glm::vec3 a = corners[triangle_table_cube[4][3]];
                                glm::vec3 b = corners[triangle_table_cube[4][4]];
                                glm::vec3 c = corners[triangle_table_cube[4][5]];
                                std::array<glm::vec3, 3> ver;
                                if(v.block) ver = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                                else ver = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                
                                glm::vec3 x_vec = glm::normalize(ver[1] - ver[2]);
                                glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));

                                float dot_top_corner = dot(y_vec, ver[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[3]]});
                                }

                                a = corners[triangle_table_cube[4][0]];
                                b = corners[triangle_table_cube[4][1]];
                                c = corners[triangle_table_cube[4][2]];
                                std::array<glm::vec3, 3> ver2;
                                if(v.block) ver2 = {glm::vec3(-1, 0, 0) + glm::vec3(x, y, z) + a, glm::vec3(-1, 0, 0) + glm::vec3(x, y, z) + b, glm::vec3(-1, 0, 0) + glm::vec3(x, y, z) + c};
                                else ver2 = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                normal = glm::normalize(glm::cross(ver2[1] - ver2[0], ver2[2] - ver2[0]));

                                

                                x_vec = glm::normalize(x_vec - normal * dot(normal, x_vec));
                                y_vec = glm::normalize(glm::cross(normal, x_vec));

                                dot_top_corner = dot(y_vec, ver2[2]);
                                
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = ver2[l];
                                    glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position) - dot_top_corner};
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[3]]});
                                }
                            } else {
                                for(int k = 0; k < 2; ++k) {
                                    glm::vec3 a = corners[triangle_table_cube[4][k * 3 + 0]];
                                    glm::vec3 b = corners[triangle_table_cube[4][k * 3 + 1]];
                                    glm::vec3 c = corners[triangle_table_cube[4][k * 3 + 2]];
                                    std::array<glm::vec3, 3> ver;
                                    if(v.block) ver = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                                    else ver = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                                    glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                                    
                                    glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                                    glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                                    if(isnan(x_vec.x)) {
                                        if(normal.z > 0) {
                                            x_vec = {1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        } else {
                                            x_vec = {-1, 0, 0};
                                            y_vec = {0, 1, 0};
                                        }
                                    }
                                    
                                    for(int l = 0; l < 3; ++l) {
                                        glm::vec3 position = ver[l];
                                        glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                        vertices.push_back({position, normal, tex_coord, v.texture[v.pt[3]]});
                                    }
                                }
                            }
                        }
                    }
                    if(voxels[22] == 0) {
                        for(int k = 0; k < 2; ++k) {
                            glm::vec3 a = corners[triangle_table_cube[5][k * 3 + 0]];
                            glm::vec3 b = corners[triangle_table_cube[5][k * 3 + 1]];
                            glm::vec3 c = corners[triangle_table_cube[5][k * 3 + 2]];
                            std::array<glm::vec3, 3> ver;
                            if(v.block) ver = {glm::vec3(x, y, z) + a, glm::vec3(x, y, z) + b, glm::vec3(x, y, z) + c};
                            else ver = {sn[x + a.x][y + a.y][z + a.z], sn[x + b.x][y + b.y][z + b.z], sn[x + c.x][y + c.y][z + c.z]};
                            glm::vec3 normal = glm::normalize(glm::cross(ver[1] - ver[0], ver[2] - ver[0]));
                            
                            glm::vec3 x_vec = glm::normalize(glm::cross(normal, -core.z_dir));
                            glm::vec3 y_vec = glm::normalize(glm::cross(normal, x_vec));
                            if(isnan(x_vec.x)) {
                                if(normal.z > 0) {
                                    x_vec = {1, 0, 0};
                                    y_vec = {0, 1, 0};
                                } else {
                                    x_vec = {-1, 0, 0};
                                    y_vec = {0, 1, 0};
                                }
                            }
                            
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = ver[l];
                                glm::vec2 tex_coord = {dot(x_vec, position), dot(y_vec, position)};
                                vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                            }   
                        }
                    }
                }
            }
        }
    }

    std::array<std::array<std::array<std::array<uint16_t, 32>, 32>, 32>*, 27> arr27v = {};

    for(int x = -1; x < 2; ++x) {
        for(int y = -1; y < 2; ++y) {
            for(int z = -1; z < 2; ++z) {
                arr27v[(x + 1) + (y + 1) * 3 + (z + 1) * 9] = &(*core.chunks[index + glm::ivec3{x, y, z}].voxels.get());
            }
        }
    }

    for(uint8_t x = 0; x < 0x20; ++x) {
        for(uint8_t y = 0; y < 0x20; ++y) {
            for(uint8_t z = 0; z < 0x20; ++z) {
                glm::vec3 pos = {x, y, z};
                
                std::array<uint16_t, 27> voxels = get_surrounding_voxels({x, y, z}, arr27v);

                if(voxels[13] == 0) {
                    if(voxels[14] != 0) {
                        voxel_info& v = voxel_data[voxels[14]];

                        glm::vec3 normal = glm::vec3(-1, 0, 0);
                        
                        
                        if(v.spread && voxel_data[voxels[4]].spread) {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + glm::vec3(1, 0, 0) + corners[triangle_table_cube[0][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(-position.y, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }   
                            }
                        } else {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + glm::vec3(1, 0, 0) + corners[triangle_table_cube[0][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(-position.y, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[0]]});
                                }   
                            }
                        }
                    }
                    if(voxels[16] != 0) {
                        voxel_info& v = voxel_data[voxels[16]];

                        glm::vec3 normal = glm::vec3(0, -1, 0);

                        if(v.spread && voxel_data[voxels[4]].spread) {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + glm::vec3(0, 1, 0) + corners[triangle_table_cube[1][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(position.x, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }   
                            }
                        } else {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + glm::vec3(0, 1, 0) + corners[triangle_table_cube[1][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(position.x, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[2]]});
                                }   
                            }
                        }
                    }
                    if(voxels[22] != 0) {
                        voxel_info& v = voxel_data[voxels[22]];

                        glm::vec3 normal = glm::vec3(0, 0, -1);

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + glm::vec3(0, 0, 1) + corners[triangle_table_cube[2][k * 3 + l]];
                                position = position * 0.5f;

                                glm::vec2 tex_coord = glm::vec2(position.x, -position.y);
                                vertices.push_back({position, normal, tex_coord, v.texture[v.pt[4]]});
                            }   
                        }
                    }
                } else {
                    voxel_info& v = voxel_data[voxels[13]];

                    if(voxels[14] == 0) {
                        glm::vec3 normal = glm::vec3(1, 0, 0);
                        
                        
                        if(v.spread && voxel_data[voxels[5]].spread) {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + corners[triangle_table_cube[3][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(position.y, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }   
                            }
                        } else {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + corners[triangle_table_cube[3][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(position.y, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[1]]});
                                }   
                            }
                        }
                    }
                    if(voxels[16] == 0) {
                        glm::vec3 normal = glm::vec3(0, 1, 0);
                        
                        if(v.spread && voxel_data[voxels[7]].spread) {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + corners[triangle_table_cube[4][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(-position.x, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                                }   
                            }
                        } else {
                            for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                                for(int l = 0; l < 3; ++l) {
                                    glm::vec3 position = pos + corners[triangle_table_cube[4][k * 3 + l]];
                                    position = position * 0.5f;

                                    glm::vec2 tex_coord = glm::vec2(-position.x, position.z);
                                    vertices.push_back({position, normal, tex_coord, v.texture[v.pt[3]]});
                                }   
                            }
                        }
                    }
                    if(voxels[22] == 0) {
                        glm::vec3 normal = glm::vec3(0, 0, 1);

                        for(int k = 0; k < triangle_table_cube[0].size() / 3; ++k) {
                            for(int l = 0; l < 3; ++l) {
                                glm::vec3 position = pos + corners[triangle_table_cube[5][k * 3 + l]];
                                position = position * 0.5f;

                                glm::vec2 tex_coord = glm::vec2(position.x, position.y);
                                vertices.push_back({position, normal, tex_coord, v.texture[v.pt[5]]});
                            }   
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

uint16_t& get_terrain_voxel(glm::ivec3 pos) {
    glm::ivec3 key = glm::floor((glm::vec3)pos / 16.0f);
    if(core.chunks.contains(key)) {
        return (*core.chunks[key].terrain_voxels)[mod(pos.x, 16)][mod(pos.y, 16)][mod(pos.z, 16)];
    } else {
        return null_ref;
    }
}

uint16_t& get_voxel(glm::ivec3 pos) {
    glm::ivec3 key = glm::floor((glm::vec3)pos / 32.0f);
    if(core.chunks.contains(key)) {
        return (*core.chunks[key].voxels)[mod(pos.x, 32)][mod(pos.y, 32)][mod(pos.z, 32)];
    } else {
        return null_ref;
    }
}

bool get_terrain_voxel(glm::ivec3 pos, uint16_t& output) {
    glm::ivec3 key = glm::floor((glm::vec3)pos / 16.0f);
    if(core.chunks.contains(key)) {
        output = (*core.chunks[key].terrain_voxels)[mod(pos.x, 16)][mod(pos.y, 16)][mod(pos.z, 16)];
        return true;
    } else {
        return false;
    }
}

bool get_voxel(glm::ivec3 pos, uint16_t& output) {
    glm::ivec3 key = glm::floor((glm::vec3)pos / 32.0f);
    if(core.chunks.contains(key)) {
        output = (*core.chunks[key].voxels)[mod(pos.x, 32)][mod(pos.y, 32)][mod(pos.z, 32)];
        return true;
    } else {
        return false;
    }
}

uint16_t& get_terrain_voxel(glm::ivec3 chunk_pos, glm::ivec3 block_pos) {
    if(core.chunks.contains(chunk_pos)) {
        return (*core.chunks[chunk_pos].terrain_voxels)[block_pos.x][block_pos.y][block_pos.z];
    } else {
        return null_ref;
    }
}

void block_update(glm::ivec3 pos) {
    glm::ivec3 i = glm::floor((glm::vec3)pos / 32.0f);
    glm::ivec3 block_i = mod(pos, 32);
    uint16_t& focus_block = get_terrain_voxel(i, block_i);
    if(focus_block == 3 || focus_block == 4 || focus_block == 5) {
        uint16_t above = get_terrain_voxel(pos + glm::ivec3{0, 0, 1});
        glm::ivec3 below_coord = pos + glm::ivec3{0, 0, -1};
        uint16_t& below = get_terrain_voxel(below_coord);
        glm::ivec3 north_coord = pos + glm::ivec3{0, 1, 0};
        uint16_t& north = get_terrain_voxel(north_coord);
        glm::ivec3 east_coord = pos + glm::ivec3{1, 0, 0};
        uint16_t& east = get_terrain_voxel(east_coord);
        glm::ivec3 south_coord = pos + glm::ivec3{0, -1, 0};
        uint16_t& south = get_terrain_voxel(south_coord);
        glm::ivec3 west_coord = pos + glm::ivec3{-1, 0, 0};
        uint16_t& west = get_terrain_voxel(west_coord);

        uint16_t below_north = get_terrain_voxel(north_coord + glm::ivec3{0, 0, -1});
        uint16_t below_east = get_terrain_voxel(east_coord + glm::ivec3{0, 0, -1});
        uint16_t below_south = get_terrain_voxel(south_coord + glm::ivec3{0, 0, -1});
        uint16_t below_west = get_terrain_voxel(west_coord + glm::ivec3{0, 0, -1});

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

bool raycast(glm::vec3 pos, glm::vec3 dir, glm::ivec3& output, bool& is_small, float limit) {
    glm::vec3 dir_norm = glm::normalize(dir);
    
    glm::ivec3 current_voxel = glm::floor(pos);
    int step_x = (dir.x < 0) ? -1 : 1;
    int step_y = (dir.y < 0) ? -1 : 1;
    int step_z = (dir.z < 0) ? -1 : 1;

    float large_dist = 0.0f;
    float small_dist = 0.0f;
    glm::ivec3 large_coord;
    glm::ivec3 small_coord;
    glm::vec3 large_pos = pos;
    glm::vec3 small_pos = pos * 2.0f;

    while(true) {
        if(large_dist > limit) {
            large_dist = NAN;
            break;
        }
        
        uint16_t block;
        bool status = get_terrain_voxel(current_voxel, block);
        if(!status) {
            large_dist = NAN;
            break;
        }

        if(block != 0) {
            large_coord = current_voxel;
            break;
        }

        float t_x = (float(current_voxel.x + !(step_x >> 31)) - large_pos.x) / dir_norm.x;
        float t_y = (float(current_voxel.y + !(step_y >> 31)) - large_pos.y) / dir_norm.y;
        float t_z = (float(current_voxel.z + !(step_z >> 31)) - large_pos.z) / dir_norm.z;

        if(t_x < t_y && t_x < t_z) {
            large_pos += dir_norm * t_x;
            large_dist += t_x;
            current_voxel.x += step_x;
        } else if(t_y < t_x && t_y < t_z) {
            large_pos += dir_norm * t_y;
            large_dist += t_y;
            current_voxel.y += step_y;
        } else {
            large_pos += dir_norm * t_z;
            large_dist += t_z;
            current_voxel.z += step_z;
        }
    }

    
    current_voxel = glm::floor(pos * 2.0f);

    while(true) {
        if(small_dist > limit * 2) {
            small_dist = NAN;
            break;
        }
        
        uint16_t block;
        bool status = get_voxel(current_voxel, block);
        if(status == false) {
            small_dist = NAN;
            break;
        }
        
        if(block != 0) {
            small_coord = current_voxel;
            break;
        }

        float t_x = (float(current_voxel.x + !(step_x >> 31)) - small_pos.x) / dir_norm.x;
        float t_y = (float(current_voxel.y + !(step_y >> 31)) - small_pos.y) / dir_norm.y;
        float t_z = (float(current_voxel.z + !(step_z >> 31)) - small_pos.z) / dir_norm.z;

        if(t_x < t_y && t_x < t_z) {
            small_pos += dir_norm * t_x;
            small_dist += t_x;
            current_voxel.x += step_x;
        } else if(t_y < t_x && t_y < t_z) {
            small_pos += dir_norm * t_y;
            small_dist += t_y;
            current_voxel.y += step_y;
        } else {
            small_pos += dir_norm * t_z;
            small_dist += t_z;
            current_voxel.z += step_z;
        }
    }

    if(isnan(large_dist)) {
        if(isnan(small_dist)) {
            return false;
        } else {
            output = small_coord;
            is_small = true;
            return true;
        }
    } else if(isnan(small_dist) || large_dist < (small_dist * 0.5)) {
        output = large_coord;
        is_small = false;
        return true;
    } else {
        output = small_coord;
        is_small = true;
        return true;
    }
}

bool raycast_place(glm::vec3 pos, glm::vec3 dir, glm::ivec3& output, bool is_small, float limit) {
    glm::vec3 dir_norm = glm::normalize(dir);
    
    glm::ivec3 current_voxel = glm::floor(pos * 2.0f);
    int step_x = (dir.x < 0) ? -1 : 1;
    int step_y = (dir.y < 0) ? -1 : 1;
    int step_z = (dir.z < 0) ? -1 : 1;

    float dist = 0.0;
    glm::ivec3 prev_coord = ((is_small) ? glm::floor(pos * 2.0f) : glm::floor(pos));
    glm::vec3 start_pos = pos * 2.0f;
    
    current_voxel = glm::floor(pos * 2.0f);

    while(true) {
        if(dist > limit * 2) {
            dist = NAN;
            break;
        }
        
        uint16_t small_block;
        bool status = get_voxel(current_voxel, small_block);
        if(status == false) {
            dist = NAN;
            break;
        }
        uint16_t large_block = get_terrain_voxel(glm::floor(glm::vec3(current_voxel) * 0.5f));
        
        if(small_block == 0 && large_block == 0) {
            if(is_small) {
                prev_coord = current_voxel;
            } else {
                prev_coord = glm::floor(glm::vec3(current_voxel) * 0.5f);
            }
        } else {
            break;
        }

        float t_x = (float(current_voxel.x + !(step_x >> 31)) - start_pos.x) / dir_norm.x;
        float t_y = (float(current_voxel.y + !(step_y >> 31)) - start_pos.y) / dir_norm.y;
        float t_z = (float(current_voxel.z + !(step_z >> 31)) - start_pos.z) / dir_norm.z;

        if(t_x < t_y && t_x < t_z) {
            start_pos += dir_norm * t_x;
            dist += t_x;
            current_voxel.x += step_x;
        } else if(t_y < t_x && t_y < t_z) {
            start_pos += dir_norm * t_y;
            dist += t_y;
            current_voxel.y += step_y;
        } else {
            start_pos += dir_norm * t_z;
            dist += t_z;
            current_voxel.z += step_z;
        }
    }

    if(isnan(dist)) {
        return false;
    } else {
        if(is_small) {
            output = prev_coord;
            return true;
        } else {
            bool contains_tile = false;
            for(int x = 0; x < 2; ++x) {
                for(int y = 0; y < 2; ++y) {
                    for(int z = 0; z < 2; ++z) {
                        if(get_voxel(prev_coord * 2 + glm::ivec3(x, y, z))) {
                            contains_tile = true;
                            goto fin;
                        }
                    }
                }
            }
            fin:

            if(!contains_tile) {
                output = prev_coord;
                return true;
            } else {
                return false;
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

//   6   7
// 4   5
//   2   3
// 0   1

std::vector<std::map<uint8_t, uint8_t>> m = {
    {
        {1, 0},
        {2, 8},
        {4, 3}
    },
    {
        {3, 9},
        {5, 1}
    },
    {
        {3, 4},
        {6, 7}
    },
    {
        {7, 5}
    },
    {
        {5, 2},
        {6, 11}
    },
    {
        {7, 10}
    },
    {
        {7, 6}
    }
};

int count = 0;

void Core::init() {
    /*for(int i = 0; i < 33; ++i) {
        print_vec({0, 0, i});
        print_vec({0, 32, i});
    }
    for(int i = 0; i < 33; ++i) {
        print_vec({0, i, 0});
        print_vec({0, i, 32});
    }

    for(int i = 0; i < 33; ++i) {
        print_vec({32, 0, i});
        print_vec({32, 32, i});
    }
    for(int i = 0; i < 33; ++i) {
        print_vec({32, i, 0});
        print_vec({32, i, 32});
    }

    for(int i = 0; i < 33; ++i) {
        print_vec({i, 0, 0});
        print_vec({i, 0, 32});
    }
    for(int i = 0; i < 33; ++i) {
        print_vec({0, 0, i});
        print_vec({32, 0, i});
    }

    for(int i = 0; i < 33; ++i) {
        print_vec({i, 32, 0});
        print_vec({i, 32, 32});
    }
    for(int i = 0; i < 33; ++i) {
        print_vec({0, 32, i});
        print_vec({32, 32, i});
    }

    for(int i = 0; i < 33; ++i) {
        print_vec({i, 0, 0});
        print_vec({i, 32, 0});
    }
    for(int i = 0; i < 33; ++i) {
        print_vec({0, i, 0});
        print_vec({32, i, 0});
    }
    
    for(int i = 0; i < 33; ++i) {
        print_vec({i, 0, 32});
        print_vec({i, 32, 32});
    }
    for(int i = 0; i < 33; ++i) {
        print_vec({0, i, 32});
        print_vec({32, i, 32});
    }

    std::cout << count;*/



    /*for(uint16_t i = 0; i <= 0xFF; ++i) {
        for(int j = 0; j < 7; ++j) {
            for(auto [vox, edge] : m[j]) {
                if((i >> vox) & 1) triangle_table[i].push_back(edge);
            }
        }
    }*/
    /*for(uint16_t i = 0; i <= 0xFF; ++i) {
        uint8_t j = 0;

        for(uint8_t x = 0; x < 8; ++x) {
            uint8_t y = x;
            if(y == 2) y = 4;
            else if(y == 3) y = 5;
            else if(y == 4) y = 3;
            else if(y == 5) y = 2;
            else if(y == 6) y = 7;
            else if(y == 7) y = 6;

            if((i >> y) & 1) {
                j |= (1 << x);
            }
        }

        std::cout << "\tstd::vector<uint8_t>{";
        for(int m = 0; m < triangle_table[j].size(); ++m) {
            uint16_t v = triangle_table[j][m];
            std::cout << v;

            if(m != triangle_table[j].size() - 1) std::cout << ", ";
        }
        std::cout << "},\n";
    }*/



    screen_matrix = glm::translate(glm::inverse(glm::scale(identity_matrix_3, glm::vec2(viewport_size / 2))), glm::vec2(-viewport_size / 2));
    gui_framebuffer.init(viewport_size.x, viewport_size.y);
    space_core.framebuffer.init(viewport_size.x, viewport_size.y);

    grid_shader.compile("res/shaders/grid_3d.vs", "res/shaders/grid_3d_xy.fs");
    chunk_shader.compile("res/shaders/chunk_3d.vs", "res/shaders/chunk_3d.fs");
    screen_shader.compile("res/shaders/screen.vs", "res/shaders/screen.fs");
    screen_shader_solid.compile("res/shaders/screen_solid.vs", "res/shaders/screen_solid.fs");
    cube_shader.compile("res/shaders/cube.vs", "res/shaders/cube.fs");
    billboard_shader.compile("res/shaders/billbd.vs", "res/shaders/billbd.fs");
    chunk_debug_shader.compile("res/shaders/chunk_debug.vs", "res/shaders/chunk_debug.fs");
    any_shader.compile("res/shaders/any.vs", "res/shaders/any.fs");

    gui_core.init();

    fnfractal->SetSource(fnperlin);
    fnfractal->SetOctaveCount(3);
    fnbiome->SetOctaveCount(3);

    tex.load("res/tilex.png", true);
    x_tex.load("res/character.png");

    //space_core.init();

    chunk_thread = std::thread(
        []() {
            chunk_load_func();
        }
    );
}

struct aabb {
    glm::vec3 pos;
    glm::vec3 size;
};

struct hexahedron {
    std::array<glm::vec3, 8> vertices;
};

bool check_collisions(hexahedron a, hexahedron b, glm::vec3& mtv) {
    glm::vec3 mtv_norm;
    float mtv_dist = __FLT_MAX__;

    std::array<glm::vec3, 8>& va = a.vertices;
    std::array<glm::vec3, 8>& vb = b.vertices;

    std::vector<glm::vec3> norms = {
        glm::normalize(va[0] - va[1]),
        glm::normalize(va[0] - va[2]),
        glm::normalize(va[0] - va[4])
    };
    std::vector<glm::vec3> axes;
    std::vector<glm::vec3> edges;

    for(auto& w : triangle_table_cube) {
        uint8_t av = w[0];
        uint8_t bv = w[1];
        uint8_t cv = w[2];

        uint8_t dv = w[3];
        uint8_t ev = w[4];
        uint8_t fv = w[5];

        glm::vec3 norm_abc = glm::normalize(glm::cross(vb[bv] - vb[av], vb[cv] - vb[av]));
        glm::vec3 norm_def = glm::normalize(glm::cross(vb[ev] - vb[dv], vb[fv] - vb[dv]));

        axes.push_back(glm::normalize(norm_abc + norm_def));
    }

    edges = {
        glm::normalize(vb[0] - vb[1]),
        glm::normalize(vb[0] - vb[2]),
        glm::normalize(vb[0] - vb[4]),
        glm::normalize(vb[1] - vb[3]),
        glm::normalize(vb[1] - vb[5]),
        glm::normalize(vb[2] - vb[3]),
        glm::normalize(vb[2] - vb[6]),
        glm::normalize(vb[3] - vb[7]),
        glm::normalize(vb[4] - vb[5]),
        glm::normalize(vb[4] - vb[6]),
        glm::normalize(vb[5] - vb[7]),
        glm::normalize(vb[6] - vb[7])
    };

    for(glm::vec3 n : norms) {
        axes.push_back(n);
        for(glm::vec3 m : edges) {
            glm::vec3 cross = glm::normalize(glm::cross(n, m));
            axes.push_back(cross);
        }
    }
    
    for(glm::vec3 n : axes) {
        if(!(isnan(n.x) || isinf(n.x))) {
            std::array<double, 2> minmax_a = {__FLT_MAX__, -__FLT_MAX__};
            std::array<double, 2> minmax_b = {__FLT_MAX__, -__FLT_MAX__};

            for(glm::vec3 v : va) {
                double z = dot(v, n);

                if(z < minmax_a[0]) minmax_a[0] = z;
                if(z > minmax_a[1]) minmax_a[1] = z;
            }

            for(glm::vec3 v : vb) {
                double z = dot(v, n);

                if(z < minmax_b[0]) minmax_b[0] = z;
                if(z > minmax_b[1]) minmax_b[1] = z;
            }

            if(minmax_a[0] < minmax_b[1] && minmax_a[1] > minmax_b[0]) {
                float dist_0 = minmax_b[1] - minmax_a[0];
                float dist_1 = minmax_b[0] - minmax_a[1];
                float new_mtv_dist = (abs(dist_0) < abs(dist_1)) ? dist_0 : dist_1;
                if(abs(mtv_dist) > abs(new_mtv_dist)) {
                    mtv_dist = new_mtv_dist;
                    mtv_norm = n;
                }
            } else {
                return false;
            }
        }
    }
    mtv = mtv_norm * mtv_dist;
    return true;
}

bool check_collisions_obb(hexahedron a, hexahedron b, glm::vec3& mtv) {
    glm::vec3 mtv_norm;
    float mtv_dist = __FLT_MAX__;

    std::array<glm::vec3, 8>& va = a.vertices;
    std::array<glm::vec3, 8>& vb = b.vertices;

    std::vector<glm::vec3> norms_a = {
        glm::normalize(va[0] - va[1]),
        glm::normalize(va[0] - va[2]),
        glm::normalize(va[0] - va[4])
    };

    std::vector<glm::vec3> norms_b = {
        glm::normalize(vb[0] - vb[1]),
        glm::normalize(vb[0] - vb[2]),
        glm::normalize(vb[0] - vb[4])
    };

    std::vector<glm::vec3> axes;

    for(auto& w : triangle_table_cube) {
        uint8_t av = w[0];
        uint8_t bv = w[1];
        uint8_t cv = w[2];

        uint8_t dv = w[3];
        uint8_t ev = w[4];
        uint8_t fv = w[5];

        glm::vec3 norm_abc = glm::normalize(glm::cross(vb[bv] - vb[av], vb[cv] - vb[av]));
        glm::vec3 norm_def = glm::normalize(glm::cross(vb[ev] - vb[dv], vb[fv] - vb[dv]));

        axes.push_back(glm::normalize(norm_abc + norm_def));
    }

    for(uint8_t i = 0; i < 3; ++i) {

        glm::vec3 n = norms_a[i];
        axes.push_back(n);
        axes.push_back(norms_b[i]);
        for(glm::vec3 m : norms_b) {
            glm::vec3 cross = glm::normalize(glm::cross(n, m));
            axes.push_back(cross);
        }
    }
    
    for(glm::vec3 n : axes) {
        if(!(isnan(n.x) || isinf(n.x))) {
            std::array<double, 2> minmax_a = {__FLT_MAX__, -__FLT_MAX__};
            std::array<double, 2> minmax_b = {__FLT_MAX__, -__FLT_MAX__};

            for(glm::vec3 v : va) {
                double z = dot(v, n);

                if(z < minmax_a[0]) minmax_a[0] = z;
                if(z > minmax_a[1]) minmax_a[1] = z;
            }

            for(glm::vec3 v : vb) {
                double z = dot(v, n);

                if(z < minmax_b[0]) minmax_b[0] = z;
                if(z > minmax_b[1]) minmax_b[1] = z;
            }

            if(minmax_a[0] < minmax_b[1] && minmax_a[1] > minmax_b[0]) {
                float dist_0 = minmax_b[1] - minmax_a[0];
                float dist_1 = minmax_b[0] - minmax_a[1];
                float new_mtv_dist = (abs(dist_0) < abs(dist_1)) ? dist_0 : dist_1;
                if(abs(mtv_dist) > abs(new_mtv_dist)) {
                    mtv_dist = new_mtv_dist;
                    mtv_norm = n;
                }
            } else {
                return false;
            }
        }
    }
    mtv = mtv_norm * mtv_dist;
    return true;
}

bool check_collisions(hexahedron a, hexahedron b, glm::vec3& mtv, glm::vec3& contact) {
    glm::vec3 mtv_norm;
    float mtv_dist = __FLT_MAX__;

    std::array<glm::vec3, 8>& va = a.vertices;
    std::array<glm::vec3, 8>& vb = b.vertices;

    std::vector<glm::vec3> norms = {
        glm::normalize(va[0] - va[1]),
        glm::normalize(va[0] - va[2]),
        glm::normalize(va[0] - va[4])
    };
    std::vector<glm::vec3> axes;
    std::vector<glm::vec3> edges;

    for(auto& w : triangle_table_cube) {
        uint8_t av = w[0];
        uint8_t bv = w[1];
        uint8_t cv = w[2];

        uint8_t dv = w[3];
        uint8_t ev = w[4];
        uint8_t fv = w[5];

        glm::vec3 norm_abc = glm::normalize(glm::cross(vb[bv] - vb[av], vb[cv] - vb[av]));
        glm::vec3 norm_def = glm::normalize(glm::cross(vb[ev] - vb[dv], vb[fv] - vb[dv]));

        axes.push_back(glm::normalize(norm_abc + norm_def));
    }

    edges = {
        glm::normalize(vb[0] - vb[1]),
        glm::normalize(vb[0] - vb[2]),
        glm::normalize(vb[0] - vb[4]),
        glm::normalize(vb[1] - vb[3]),
        glm::normalize(vb[1] - vb[5]),
        glm::normalize(vb[2] - vb[3]),
        glm::normalize(vb[2] - vb[6]),
        glm::normalize(vb[3] - vb[7]),
        glm::normalize(vb[4] - vb[5]),
        glm::normalize(vb[4] - vb[6]),
        glm::normalize(vb[5] - vb[7]),
        glm::normalize(vb[6] - vb[7])
    };

    for(glm::vec3 n : norms) {
        axes.push_back(n);
        for(glm::vec3 m : edges) {
            glm::vec3 cross = glm::normalize(glm::cross(n, m));
            axes.push_back(cross);
        }
    }
    
    for(glm::vec3 n : axes) {
        if(!(isnan(n.x) || isinf(n.x))) {
            std::array<double, 2> minmax_a = {__FLT_MAX__, -__FLT_MAX__};
            std::array<double, 2> minmax_b = {__FLT_MAX__, -__FLT_MAX__};

            for(glm::vec3 v : va) {
                double z = dot(v, n);

                if(z < minmax_a[0]) minmax_a[0] = z;
                if(z > minmax_a[1]) minmax_a[1] = z;
            }

            for(glm::vec3 v : vb) {
                double z = dot(v, n);

                if(z < minmax_b[0]) minmax_b[0] = z;
                if(z > minmax_b[1]) minmax_b[1] = z;
            }

            if(minmax_a[0] < minmax_b[1] && minmax_a[1] > minmax_b[0]) {
                float dist_0 = minmax_b[1] - minmax_a[0];
                float dist_1 = minmax_b[0] - minmax_a[1];
                float new_mtv_dist = (abs(dist_0) < abs(dist_1)) ? dist_0 : dist_1;
                if(abs(mtv_dist) > abs(new_mtv_dist)) {
                    mtv_dist = new_mtv_dist;
                    mtv_norm = n;
                }
            } else {
                return false;
            }
        }
    }
    mtv = mtv_norm * mtv_dist;
    return true;
}

bool check_collisions(aabb a, hexahedron b) {
    glm::vec3 mtv_norm;
    float mtv_dist = __FLT_MAX__;

    std::vector<glm::vec3> va;
    std::array<glm::vec3, 8>& vb = b.vertices;

    for(int i = 0; i < 8; ++i) {
        glm::vec3 iv = {i & 1, (i >> 1) & 1, i >> 2};
        va.push_back(a.pos + a.size * iv);
    }

    std::vector<glm::vec3> norms = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };
    std::vector<glm::vec3> axes;
    std::vector<glm::vec3> edges;

    for(auto& w : triangle_table_cube) {
        uint8_t av = w[0];
        uint8_t bv = w[1];
        uint8_t cv = w[2];

        glm::vec3 norm_abc = glm::normalize(glm::cross(vb[bv] - vb[av], vb[cv] - vb[av]));

        axes.push_back(norm_abc);
    }

    edges = {
        glm::normalize(vb[0] - vb[1]),
        glm::normalize(vb[0] - vb[2]),
        glm::normalize(vb[0] - vb[4]),
        glm::normalize(vb[1] - vb[3]),
        glm::normalize(vb[1] - vb[5]),
        glm::normalize(vb[2] - vb[3]),
        glm::normalize(vb[2] - vb[6]),
        glm::normalize(vb[3] - vb[7]),
        glm::normalize(vb[4] - vb[5]),
        glm::normalize(vb[4] - vb[6]),
        glm::normalize(vb[5] - vb[7]),
        glm::normalize(vb[6] - vb[7])
    };

    for(glm::vec3 n : norms) {
        axes.push_back(n);
        for(glm::vec3 m : edges) {
            glm::vec3 cross = glm::normalize(glm::cross(n, m));
            axes.push_back(cross);
        }
    }

    std::vector<float> collector;
    
    for(glm::vec3 n : axes) {
        if(!(isnan(n.x) || isinf(n.x))) {
            std::array<float, 2> minmax_a = {__FLT_MAX__, -__FLT_MAX__};
            std::array<float, 2> minmax_b = {__FLT_MAX__, -__FLT_MAX__};

            for(glm::vec3 v : va) {
                float z = dot(v, n);

                if(z < minmax_a[0]) minmax_a[0] = z;
                if(z > minmax_a[1]) minmax_a[1] = z;
            }

            for(glm::vec3 v : vb) {
                float z = dot(v, n);
                collector.push_back(z);

                if(z < minmax_b[0]) minmax_b[0] = z;
                if(z > minmax_b[1]) minmax_b[1] = z;
            }

            if(!(minmax_a[0] < minmax_b[1] && minmax_a[1] > minmax_b[0])) {
                /*std::cout << n.x << " " << n.y << " " << n.z << "\n";
                std::cout << minmax_a[0] << " " << minmax_a[1] << " " << minmax_b[0] << " " << minmax_b[1] << "\n";

                for(float f : collector) {
                    std::cout << f << " ";
                }
                std::cout << "\n\n";*/

                return false;
            }
        }
    }
    return true;
}