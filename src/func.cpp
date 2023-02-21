#pragma once

#include "wrapper.cpp"

struct AABB {
    glm::ivec2 size;
    glm::ivec2 position;

    bool contains(glm::ivec2 point) {
        return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y && point.y <= position.y + size.y;
    }

    bool contains(AABB box) {
        return box.position.x + box.size.x >= position.x && box.position.x <= position.x + size.x && box.position.y + box.size.y >= position.y && box.position.y <= position.y + size.y;
    }
};