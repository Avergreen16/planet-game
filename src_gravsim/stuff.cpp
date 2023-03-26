#include "wrapper.cpp"

int from_density(double density, double x, double y, double z) {
    return (4.0/3) * M_PI * x * y * z * density;
}

double G = 0.0001;
double simulation_speed = 1;
bool sim_paused = true;

int trail_lim = 0x20;
double trail_buffer_counter = 0;

double quadtree_rad = 0x10000;

double core_x_dist = 0x2000;
double core_y_dist = 0x2000;
double core_z_dist = 0x1400;
int stars_in_core = 0x0; // 1400 <- 2000

double arm_x_dist = 0x40; // 1E00
double arm_y_dist = 0x20; // 20,971,520
double arm_z_dist = 0x4;
double arm_x_mean = 0x0; // <- A800
double arm_x_ext = 0x0;
double arm_y_mean = 0x0;
double arm_x_offset = 0x0;
// x_mean should be x_dist * 0x1.8 + x_offset for a barred spiral galaxy
int stars_per_arm = from_density(0x0.4p0, arm_x_dist, arm_y_dist, arm_z_dist);
double spiral = 0x2.0p0;
int arms = 3;

double outer_core_x_dist = 0x3800;
double outer_core_y_dist = 0x1800;
double outer_core_z_dist = 0x2C00;
int stars_in_outer_core = 0x0; // <- 600

std::vector<std::pair<glm::vec4, int>> clusters = {};/*
    {glm::vec4{0x3C00, 0x1400, 0x5000, 0x80}, 0x20},
    {glm::vec4{-0x2800, 0x6400, -0x2800, 0xC0}, 0x30},
    {glm::vec4{-0x1000, -0x5800, 0x1800, 0x68}, 0x28},
    {glm::vec4{0x1000, -0x7000, -0x4000, 0x100}, 0x40},
    {glm::vec4{0x2000, 0x8000, 0xC00, 0x90}, 0x2C},
    {glm::vec4{0, 0, 0, 0x180}, 0x400}
    //{{}, },
    //{{}, },
    //{{}, }
};*/

// C00 1600 C00 200 2800 1000 300 6200 0 1000 100 1.8 2 (1C00 1C00 1C00 C0) (barred spiral)
// 1000 1000 1000 200 2800 1000 300 5C00 0 0 100 1.8 3 (spiral)

uint64_t get_time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct Star {
    glm::vec3 position;
    glm::vec4 color;
    double luminosity = 1;
};

struct Star_node {
    glm::vec3 pos;
    float spread;
    float density;
};

std::vector<Star_node> star_nodes = {
    {{0, 0, 0}, 9, 1}
};

struct Planetoid {
    glm::vec4 color;
    glm::dvec2 position = {0, 0};
    double radius = 1;
    double mass = 1;

    glm::dvec2 velocity = {0, 0};
    glm::dvec2 accel = {0, 0};

    Buffer trail_buffer;
    std::vector<glm::vec2> trail;

    Planetoid(glm::vec4 c, glm::dvec2 p, double r, double m, glm::dvec2 v = {0, 0}) {
        color = c;
        position = p;
        radius = r;
        mass = m;
        velocity = v;

        trail_buffer.init();
        trail = {glm::vec2{position}};
    }

    Planetoid(Planetoid&& a) noexcept {
        color = a.color;
        position = a.position;
        radius = a.radius;
        mass = a.mass;
        velocity = a.velocity;

        trail = std::move(a.trail);
        std::swap(trail_buffer, a.trail_buffer);
    }

    Planetoid& operator=(Planetoid&& a) noexcept {
        color = a.color;
        position = a.position;
        radius = a.radius;
        mass = a.mass;
        velocity = a.velocity;

        trail = std::move(a.trail);
        std::swap(trail_buffer, a.trail_buffer);
        return *this;
    }

    void upgrade(Planetoid&& a) {
        color = a.color;
        position = a.position;
        radius = a.radius;
        mass = a.mass;
        velocity = a.velocity;
    }

    void update_trail_buffer() {
        trail.insert(trail.begin(), position);
        if(trail.size() > trail_lim) trail.erase(trail.begin() + trail_lim, trail.end());

        trail_buffer.set_data(&trail[0], trail.size(), sizeof(float) * 2, GL_DYNAMIC_DRAW);
        trail_buffer.set_attrib(0, 2, sizeof(float) * 2, 0);
    }

    ~Planetoid() noexcept = default;
};

float gaussian_random(float mean, float stdev) {
    float u = float(rand()) / 32768;
    float v = float(rand()) / 32768;
    
    return (sqrt(-2 * std::log(u)) * cos(2.0 * M_PI * v)) * stdev + mean;
}

void spiral_func(Star& star) {
    float r = glm::length(star.position.xy());
    double theta =  atan2(star.position.y, star.position.x);
    theta += (r / (arm_x_dist)) * spiral;

    star.position.x = r * cos(theta);
    star.position.y = r * sin(theta);

    //star.color = {1.0f, (theta > M_PI / 2) ? 1.0 : 0.0, (theta > 3 * M_PI / 2) ? 1.0 : 0.0, 1.0f};
}

/*glm::vec2 spiral_func(glm::vec2 pos, double offset) {
    float r = glm::length(pos);
    double theta = offset;
    theta += atan2(pos.y, pos.x);
    theta += (std::max(r - arm_x_ext - arm_x_offset, 0.0) / (arm_x_dist)) * spiral;

    return {r * cos(theta), r * sin(theta)};
}*/