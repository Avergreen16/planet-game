#include "wrapper.cpp"

double G = 0.0001;
double simulation_speed = 1;
bool sim_paused = true;

int trail_lim = 0x20;
double trail_buffer_counter = 0;

double quadtree_rad = 0x10000;

double core_x_dist = 0x1800;
double core_y_dist = 0x1800;
int stars_in_core = 0x100;
double arm_x_dist = 0x4000;
double arm_y_dist = 0x1000;
double arm_x_mean = 0x7000;
double arm_y_mean = 0x0;
double arm_x_offset = 0x0;
// x_mean should be x_dist * 0x1.8 + x_offset for a barred spiral galaxy
int stars_per_arm = 0x200;
double spiral = 1.5;
int arms = 3;

uint64_t get_time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

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

glm::vec2 spiral_func(glm::vec2 pos, double offset) {
    float r = glm::length(pos);
    double theta = offset;
    theta += (pos.x > 0) ? atan(pos.y / pos.x) : atan(pos.y / pos.x) + M_PI;
    theta += (std::max(r - arm_x_offset, 0.0) / (arm_x_dist)) * spiral;
    return {r * cos(theta), r * sin(theta)};
}