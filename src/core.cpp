#include "global.cpp"

struct Camera {
    glm::dvec2 pos;

    Camera(glm::dvec2 pos) {
        this->pos = pos;
    }

    glm::mat4 get_view_matrix() {
        return glm::lookAt(glm::vec3(pos, 1.0f), glm::vec3(pos, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
};