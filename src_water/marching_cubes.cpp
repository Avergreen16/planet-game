#include "includes.cpp"

const std::array<const glm::vec3, 8> corners = {
    glm::vec3{0, 0, 0},
    glm::vec3{1, 0, 0},
    glm::vec3{0, 1, 0},
    glm::vec3{1, 1, 0},
    glm::vec3{0, 0, 1},
    glm::vec3{1, 0, 1},
    glm::vec3{0, 1, 1},
    glm::vec3{1, 1, 1}
};

std::array<std::vector<uint8_t>, 6> triangle_table_cube = {
    std::vector<uint8_t>{2, 0, 4, 2, 4, 6},
    std::vector<uint8_t>{0, 1, 5, 0, 5, 4},
    std::vector<uint8_t>{2, 3, 1, 2, 1, 0},
    std::vector<uint8_t>{1, 3, 7, 1, 7, 5},
    std::vector<uint8_t>{3, 2, 6, 3, 6, 7},
    std::vector<uint8_t>{4, 5, 7, 4, 7, 6}
};

/*

  6   7
4   5
  2   3
0   1

  7   6
3   2
  4   5
0   1

*/


//{0, 1, 1, 5, 4, 5, 0, 4, 2, 3, 3, 7, 6, 7, 2, 6, 0, 2, 1, 3, 4, 6, 5, 7}


const std::array<const glm::vec3, 12> midpoint_vertices = {
    glm::vec3{0.5, 0.0, 0.0},
    glm::vec3{1.0, 0.5, 0.0},
    glm::vec3{0.5, 1.0, 0.0},
    glm::vec3{0.0, 0.5, 0.0},
    glm::vec3{0.5, 0.0, 1.0},
    glm::vec3{1.0, 0.5, 1.0},
    glm::vec3{0.5, 1.0, 1.0},
    glm::vec3{0.0, 0.5, 1.0},
    glm::vec3{0.0, 0.0, 0.5},
    glm::vec3{1.0, 0.0, 0.5},
    glm::vec3{1.0, 1.0, 0.5},
    glm::vec3{0.0, 1.0, 0.5}
};

const std::array<const std::array<uint8_t, 2>, 12> midpoint_corners = {
    std::array<uint8_t, 2>{0, 1},
    std::array<uint8_t, 2>{1, 2},
    std::array<uint8_t, 2>{2, 3},
    std::array<uint8_t, 2>{0, 3},
    std::array<uint8_t, 2>{4, 5},
    std::array<uint8_t, 2>{5, 6},
    std::array<uint8_t, 2>{6, 7},
    std::array<uint8_t, 2>{4, 7},
    std::array<uint8_t, 2>{0, 4},
    std::array<uint8_t, 2>{1, 5},
    std::array<uint8_t, 2>{2, 6},
    std::array<uint8_t, 2>{3, 7}
};

std::array<std::vector<glm::vec3>, 0x100> normal_table;

std::array<std::vector<uint8_t>, 0x100> triangle_table = {
    std::vector<uint8_t>{},
	std::vector<uint8_t>{0, 3, 8},
	std::vector<uint8_t>{0, 9, 1},
	std::vector<uint8_t>{3, 8, 1, 1, 8, 9},
	std::vector<uint8_t>{2, 11, 3},
	std::vector<uint8_t>{8, 0, 11, 11, 0, 2},
	std::vector<uint8_t>{3, 2, 11, 1, 0, 9},
	std::vector<uint8_t>{11, 1, 2, 11, 9, 1, 11, 8, 9},
	std::vector<uint8_t>{1, 10, 2},
	std::vector<uint8_t>{0, 3, 8, 2, 1, 10},
	std::vector<uint8_t>{10, 2, 9, 9, 2, 0},
	std::vector<uint8_t>{8, 2, 3, 8, 10, 2, 8, 9, 10},
	std::vector<uint8_t>{11, 3, 10, 10, 3, 1},
	std::vector<uint8_t>{10, 0, 1, 10, 8, 0, 10, 11, 8},
	std::vector<uint8_t>{9, 3, 0, 9, 11, 3, 9, 10, 11},
	std::vector<uint8_t>{8, 9, 11, 11, 9, 10},
	std::vector<uint8_t>{4, 8, 7},
	std::vector<uint8_t>{7, 4, 3, 3, 4, 0},
	std::vector<uint8_t>{4, 8, 7, 0, 9, 1},
	std::vector<uint8_t>{1, 4, 9, 1, 7, 4, 1, 3, 7},
	std::vector<uint8_t>{8, 7, 4, 11, 3, 2},
	std::vector<uint8_t>{4, 11, 7, 4, 2, 11, 4, 0, 2},
	std::vector<uint8_t>{0, 9, 1, 8, 7, 4, 11, 3, 2},
	std::vector<uint8_t>{7, 4, 11, 11, 4, 2, 2, 4, 9, 2, 9, 1},
	std::vector<uint8_t>{4, 8, 7, 2, 1, 10},
	std::vector<uint8_t>{7, 4, 3, 3, 4, 0, 10, 2, 1},
	std::vector<uint8_t>{10, 2, 9, 9, 2, 0, 7, 4, 8},
	std::vector<uint8_t>{10, 2, 3, 10, 3, 4, 3, 7, 4, 9, 10, 4},
	std::vector<uint8_t>{1, 10, 3, 3, 10, 11, 4, 8, 7},
	std::vector<uint8_t>{10, 11, 1, 11, 7, 4, 1, 11, 4, 1, 4, 0},
	std::vector<uint8_t>{7, 4, 8, 9, 3, 0, 9, 11, 3, 9, 10, 11},
	std::vector<uint8_t>{7, 4, 11, 4, 9, 11, 9, 10, 11},
	std::vector<uint8_t>{9, 4, 5},
	std::vector<uint8_t>{9, 4, 5, 8, 0, 3},
	std::vector<uint8_t>{4, 5, 0, 0, 5, 1},
	std::vector<uint8_t>{5, 8, 4, 5, 3, 8, 5, 1, 3},
	std::vector<uint8_t>{9, 4, 5, 11, 3, 2},
	std::vector<uint8_t>{2, 11, 0, 0, 11, 8, 5, 9, 4},
	std::vector<uint8_t>{4, 5, 0, 0, 5, 1, 11, 3, 2},
	std::vector<uint8_t>{5, 1, 4, 1, 2, 11, 4, 1, 11, 4, 11, 8},
	std::vector<uint8_t>{1, 10, 2, 5, 9, 4},
	std::vector<uint8_t>{9, 4, 5, 0, 3, 8, 2, 1, 10},
	std::vector<uint8_t>{2, 5, 10, 2, 4, 5, 2, 0, 4},
	std::vector<uint8_t>{10, 2, 5, 5, 2, 4, 4, 2, 3, 4, 3, 8},
	std::vector<uint8_t>{11, 3, 10, 10, 3, 1, 4, 5, 9},
	std::vector<uint8_t>{4, 5, 9, 10, 0, 1, 10, 8, 0, 10, 11, 8},
	std::vector<uint8_t>{11, 3, 0, 11, 0, 5, 0, 4, 5, 10, 11, 5},
	std::vector<uint8_t>{4, 5, 8, 5, 10, 8, 10, 11, 8},
	std::vector<uint8_t>{8, 7, 9, 9, 7, 5},
	std::vector<uint8_t>{3, 9, 0, 3, 5, 9, 3, 7, 5},
	std::vector<uint8_t>{7, 0, 8, 7, 1, 0, 7, 5, 1},
	std::vector<uint8_t>{7, 5, 3, 3, 5, 1},
	std::vector<uint8_t>{5, 9, 7, 7, 9, 8, 2, 11, 3},
	std::vector<uint8_t>{2, 11, 7, 2, 7, 9, 7, 5, 9, 0, 2, 9},
	std::vector<uint8_t>{2, 11, 3, 7, 0, 8, 7, 1, 0, 7, 5, 1},
	std::vector<uint8_t>{2, 11, 1, 11, 7, 1, 7, 5, 1},
	std::vector<uint8_t>{8, 7, 9, 9, 7, 5, 2, 1, 10},
	std::vector<uint8_t>{10, 2, 1, 3, 9, 0, 3, 5, 9, 3, 7, 5},
	std::vector<uint8_t>{7, 5, 8, 5, 10, 2, 8, 5, 2, 8, 2, 0},
	std::vector<uint8_t>{10, 2, 5, 2, 3, 5, 3, 7, 5},
	std::vector<uint8_t>{8, 7, 5, 8, 5, 9, 11, 3, 10, 3, 1, 10},
	std::vector<uint8_t>{5, 11, 7, 10, 11, 5, 1, 9, 0},
	std::vector<uint8_t>{11, 5, 10, 7, 5, 11, 8, 3, 0},
	std::vector<uint8_t>{5, 11, 7, 10, 11, 5},
	std::vector<uint8_t>{6, 7, 11},
	std::vector<uint8_t>{7, 11, 6, 3, 8, 0},
	std::vector<uint8_t>{6, 7, 11, 0, 9, 1},
	std::vector<uint8_t>{9, 1, 8, 8, 1, 3, 6, 7, 11},
	std::vector<uint8_t>{3, 2, 7, 7, 2, 6},
	std::vector<uint8_t>{0, 7, 8, 0, 6, 7, 0, 2, 6},
	std::vector<uint8_t>{6, 7, 2, 2, 7, 3, 9, 1, 0},
	std::vector<uint8_t>{6, 7, 8, 6, 8, 1, 8, 9, 1, 2, 6, 1},
	std::vector<uint8_t>{11, 6, 7, 10, 2, 1},
	std::vector<uint8_t>{3, 8, 0, 11, 6, 7, 10, 2, 1},
	std::vector<uint8_t>{0, 9, 2, 2, 9, 10, 7, 11, 6},
	std::vector<uint8_t>{6, 7, 11, 8, 2, 3, 8, 10, 2, 8, 9, 10},
	std::vector<uint8_t>{7, 10, 6, 7, 1, 10, 7, 3, 1},
	std::vector<uint8_t>{8, 0, 7, 7, 0, 6, 6, 0, 1, 6, 1, 10},
	std::vector<uint8_t>{7, 3, 6, 3, 0, 9, 6, 3, 9, 6, 9, 10},
	std::vector<uint8_t>{6, 7, 10, 7, 8, 10, 8, 9, 10},
	std::vector<uint8_t>{11, 6, 8, 8, 6, 4},
	std::vector<uint8_t>{6, 3, 11, 6, 0, 3, 6, 4, 0},
	std::vector<uint8_t>{11, 6, 8, 8, 6, 4, 1, 0, 9},
	std::vector<uint8_t>{1, 3, 9, 3, 11, 6, 9, 3, 6, 9, 6, 4},
	std::vector<uint8_t>{2, 8, 3, 2, 4, 8, 2, 6, 4},
	std::vector<uint8_t>{4, 0, 6, 6, 0, 2},
	std::vector<uint8_t>{9, 1, 0, 2, 8, 3, 2, 4, 8, 2, 6, 4},
	std::vector<uint8_t>{9, 1, 4, 1, 2, 4, 2, 6, 4},
	std::vector<uint8_t>{4, 8, 6, 6, 8, 11, 1, 10, 2},
	std::vector<uint8_t>{1, 10, 2, 6, 3, 11, 6, 0, 3, 6, 4, 0},
	std::vector<uint8_t>{11, 6, 4, 11, 4, 8, 10, 2, 9, 2, 0, 9},
	std::vector<uint8_t>{10, 4, 9, 6, 4, 10, 11, 2, 3},
	std::vector<uint8_t>{4, 8, 3, 4, 3, 10, 3, 1, 10, 6, 4, 10},
	std::vector<uint8_t>{1, 10, 0, 10, 6, 0, 6, 4, 0},
	std::vector<uint8_t>{4, 10, 6, 9, 10, 4, 0, 8, 3},
	std::vector<uint8_t>{4, 10, 6, 9, 10, 4},
	std::vector<uint8_t>{6, 7, 11, 4, 5, 9},
	std::vector<uint8_t>{4, 5, 9, 7, 11, 6, 3, 8, 0},
	std::vector<uint8_t>{1, 0, 5, 5, 0, 4, 11, 6, 7},
	std::vector<uint8_t>{11, 6, 7, 5, 8, 4, 5, 3, 8, 5, 1, 3},
	std::vector<uint8_t>{3, 2, 7, 7, 2, 6, 9, 4, 5},
	std::vector<uint8_t>{5, 9, 4, 0, 7, 8, 0, 6, 7, 0, 2, 6},
	std::vector<uint8_t>{3, 2, 6, 3, 6, 7, 1, 0, 5, 0, 4, 5},
	std::vector<uint8_t>{6, 1, 2, 5, 1, 6, 4, 7, 8},
	std::vector<uint8_t>{10, 2, 1, 6, 7, 11, 4, 5, 9},
	std::vector<uint8_t>{0, 3, 8, 4, 5, 9, 11, 6, 7, 10, 2, 1},
	std::vector<uint8_t>{7, 11, 6, 2, 5, 10, 2, 4, 5, 2, 0, 4},
	std::vector<uint8_t>{8, 4, 7, 5, 10, 6, 3, 11, 2},
	std::vector<uint8_t>{9, 4, 5, 7, 10, 6, 7, 1, 10, 7, 3, 1},
	std::vector<uint8_t>{10, 6, 5, 7, 8, 4, 1, 9, 0},
	std::vector<uint8_t>{4, 3, 0, 7, 3, 4, 6, 5, 10},
	std::vector<uint8_t>{10, 6, 5, 8, 4, 7},
	std::vector<uint8_t>{9, 6, 5, 9, 11, 6, 9, 8, 11},
	std::vector<uint8_t>{11, 6, 3, 3, 6, 0, 0, 6, 5, 0, 5, 9},
	std::vector<uint8_t>{11, 6, 5, 11, 5, 0, 5, 1, 0, 8, 11, 0},
	std::vector<uint8_t>{11, 6, 3, 6, 5, 3, 5, 1, 3},
	std::vector<uint8_t>{9, 8, 5, 8, 3, 2, 5, 8, 2, 5, 2, 6},
	std::vector<uint8_t>{5, 9, 6, 9, 0, 6, 0, 2, 6},
	std::vector<uint8_t>{1, 6, 5, 2, 6, 1, 3, 0, 8},
	std::vector<uint8_t>{1, 6, 5, 2, 6, 1},
	std::vector<uint8_t>{2, 1, 10, 9, 6, 5, 9, 11, 6, 9, 8, 11},
	std::vector<uint8_t>{9, 0, 1, 3, 11, 2, 5, 10, 6},
	std::vector<uint8_t>{11, 0, 8, 2, 0, 11, 10, 6, 5},
	std::vector<uint8_t>{3, 11, 2, 5, 10, 6},
	std::vector<uint8_t>{1, 8, 3, 9, 8, 1, 5, 10, 6},
	std::vector<uint8_t>{6, 5, 10, 0, 1, 9},
	std::vector<uint8_t>{8, 3, 0, 5, 10, 6},
	std::vector<uint8_t>{6, 5, 10},
	std::vector<uint8_t>{10, 5, 6},
	std::vector<uint8_t>{0, 3, 8, 6, 10, 5},
	std::vector<uint8_t>{10, 5, 6, 9, 1, 0},
	std::vector<uint8_t>{3, 8, 1, 1, 8, 9, 6, 10, 5},
	std::vector<uint8_t>{2, 11, 3, 6, 10, 5},
	std::vector<uint8_t>{8, 0, 11, 11, 0, 2, 5, 6, 10},
	std::vector<uint8_t>{1, 0, 9, 2, 11, 3, 6, 10, 5},
	std::vector<uint8_t>{5, 6, 10, 11, 1, 2, 11, 9, 1, 11, 8, 9},
	std::vector<uint8_t>{5, 6, 1, 1, 6, 2},
	std::vector<uint8_t>{5, 6, 1, 1, 6, 2, 8, 0, 3},
	std::vector<uint8_t>{6, 9, 5, 6, 0, 9, 6, 2, 0},
	std::vector<uint8_t>{6, 2, 5, 2, 3, 8, 5, 2, 8, 5, 8, 9},
	std::vector<uint8_t>{3, 6, 11, 3, 5, 6, 3, 1, 5},
	std::vector<uint8_t>{8, 0, 1, 8, 1, 6, 1, 5, 6, 11, 8, 6},
	std::vector<uint8_t>{11, 3, 6, 6, 3, 5, 5, 3, 0, 5, 0, 9},
	std::vector<uint8_t>{5, 6, 9, 6, 11, 9, 11, 8, 9},
	std::vector<uint8_t>{5, 6, 10, 7, 4, 8},
	std::vector<uint8_t>{0, 3, 4, 4, 3, 7, 10, 5, 6},
	std::vector<uint8_t>{5, 6, 10, 4, 8, 7, 0, 9, 1},
	std::vector<uint8_t>{6, 10, 5, 1, 4, 9, 1, 7, 4, 1, 3, 7},
	std::vector<uint8_t>{7, 4, 8, 6, 10, 5, 2, 11, 3},
	std::vector<uint8_t>{10, 5, 6, 4, 11, 7, 4, 2, 11, 4, 0, 2},
	std::vector<uint8_t>{4, 8, 7, 6, 10, 5, 3, 2, 11, 1, 0, 9},
	std::vector<uint8_t>{1, 2, 10, 11, 7, 6, 9, 5, 4},
	std::vector<uint8_t>{2, 1, 6, 6, 1, 5, 8, 7, 4},
	std::vector<uint8_t>{0, 3, 7, 0, 7, 4, 2, 1, 6, 1, 5, 6},
	std::vector<uint8_t>{8, 7, 4, 6, 9, 5, 6, 0, 9, 6, 2, 0},
	std::vector<uint8_t>{7, 2, 3, 6, 2, 7, 5, 4, 9},
	std::vector<uint8_t>{4, 8, 7, 3, 6, 11, 3, 5, 6, 3, 1, 5},
	std::vector<uint8_t>{5, 0, 1, 4, 0, 5, 7, 6, 11},
	std::vector<uint8_t>{9, 5, 4, 6, 11, 7, 0, 8, 3},
	std::vector<uint8_t>{11, 7, 6, 9, 5, 4},
	std::vector<uint8_t>{6, 10, 4, 4, 10, 9},
	std::vector<uint8_t>{6, 10, 4, 4, 10, 9, 3, 8, 0},
	std::vector<uint8_t>{0, 10, 1, 0, 6, 10, 0, 4, 6},
	std::vector<uint8_t>{6, 10, 1, 6, 1, 8, 1, 3, 8, 4, 6, 8},
	std::vector<uint8_t>{9, 4, 10, 10, 4, 6, 3, 2, 11},
	std::vector<uint8_t>{2, 11, 8, 2, 8, 0, 6, 10, 4, 10, 9, 4},
	std::vector<uint8_t>{11, 3, 2, 0, 10, 1, 0, 6, 10, 0, 4, 6},
	std::vector<uint8_t>{6, 8, 4, 11, 8, 6, 2, 10, 1},
	std::vector<uint8_t>{4, 1, 9, 4, 2, 1, 4, 6, 2},
	std::vector<uint8_t>{3, 8, 0, 4, 1, 9, 4, 2, 1, 4, 6, 2},
	std::vector<uint8_t>{6, 2, 4, 4, 2, 0},
	std::vector<uint8_t>{3, 8, 2, 8, 4, 2, 4, 6, 2},
	std::vector<uint8_t>{4, 6, 9, 6, 11, 3, 9, 6, 3, 9, 3, 1},
	std::vector<uint8_t>{8, 6, 11, 4, 6, 8, 9, 0, 1},
	std::vector<uint8_t>{11, 3, 6, 3, 0, 6, 0, 4, 6},
	std::vector<uint8_t>{8, 6, 11, 4, 6, 8},
	std::vector<uint8_t>{10, 7, 6, 10, 8, 7, 10, 9, 8},
	std::vector<uint8_t>{3, 7, 0, 7, 6, 10, 0, 7, 10, 0, 10, 9},
	std::vector<uint8_t>{6, 10, 7, 7, 10, 8, 8, 10, 1, 8, 1, 0},
	std::vector<uint8_t>{6, 10, 7, 10, 1, 7, 1, 3, 7},
	std::vector<uint8_t>{3, 2, 11, 10, 7, 6, 10, 8, 7, 10, 9, 8},
	std::vector<uint8_t>{2, 9, 0, 10, 9, 2, 6, 11, 7},
	std::vector<uint8_t>{0, 8, 3, 7, 6, 11, 1, 2, 10},
	std::vector<uint8_t>{7, 6, 11, 1, 2, 10},
	std::vector<uint8_t>{2, 1, 9, 2, 9, 7, 9, 8, 7, 6, 2, 7},
	std::vector<uint8_t>{2, 7, 6, 3, 7, 2, 0, 1, 9},
	std::vector<uint8_t>{8, 7, 0, 7, 6, 0, 6, 2, 0},
	std::vector<uint8_t>{7, 2, 3, 6, 2, 7},
	std::vector<uint8_t>{8, 1, 9, 3, 1, 8, 11, 7, 6},
	std::vector<uint8_t>{11, 7, 6, 1, 9, 0},
	std::vector<uint8_t>{6, 11, 7, 0, 8, 3},
	std::vector<uint8_t>{11, 7, 6},
	std::vector<uint8_t>{7, 11, 5, 5, 11, 10},
	std::vector<uint8_t>{10, 5, 11, 11, 5, 7, 0, 3, 8},
	std::vector<uint8_t>{7, 11, 5, 5, 11, 10, 0, 9, 1},
	std::vector<uint8_t>{7, 11, 10, 7, 10, 5, 3, 8, 1, 8, 9, 1},
	std::vector<uint8_t>{5, 2, 10, 5, 3, 2, 5, 7, 3},
	std::vector<uint8_t>{5, 7, 10, 7, 8, 0, 10, 7, 0, 10, 0, 2},
	std::vector<uint8_t>{0, 9, 1, 5, 2, 10, 5, 3, 2, 5, 7, 3},
	std::vector<uint8_t>{9, 7, 8, 5, 7, 9, 10, 1, 2},
	std::vector<uint8_t>{1, 11, 2, 1, 7, 11, 1, 5, 7},
	std::vector<uint8_t>{8, 0, 3, 1, 11, 2, 1, 7, 11, 1, 5, 7},
	std::vector<uint8_t>{7, 11, 2, 7, 2, 9, 2, 0, 9, 5, 7, 9},
	std::vector<uint8_t>{7, 9, 5, 8, 9, 7, 3, 11, 2},
	std::vector<uint8_t>{3, 1, 7, 7, 1, 5},
	std::vector<uint8_t>{8, 0, 7, 0, 1, 7, 1, 5, 7},
	std::vector<uint8_t>{0, 9, 3, 9, 5, 3, 5, 7, 3},
	std::vector<uint8_t>{9, 7, 8, 5, 7, 9},
	std::vector<uint8_t>{8, 5, 4, 8, 10, 5, 8, 11, 10},
	std::vector<uint8_t>{0, 3, 11, 0, 11, 5, 11, 10, 5, 4, 0, 5},
	std::vector<uint8_t>{1, 0, 9, 8, 5, 4, 8, 10, 5, 8, 11, 10},
	std::vector<uint8_t>{10, 3, 11, 1, 3, 10, 9, 5, 4},
	std::vector<uint8_t>{3, 2, 8, 8, 2, 4, 4, 2, 10, 4, 10, 5},
	std::vector<uint8_t>{10, 5, 2, 5, 4, 2, 4, 0, 2},
	std::vector<uint8_t>{5, 4, 9, 8, 3, 0, 10, 1, 2},
	std::vector<uint8_t>{2, 10, 1, 4, 9, 5},
	std::vector<uint8_t>{8, 11, 4, 11, 2, 1, 4, 11, 1, 4, 1, 5},
	std::vector<uint8_t>{0, 5, 4, 1, 5, 0, 2, 3, 11},
	std::vector<uint8_t>{0, 11, 2, 8, 11, 0, 4, 9, 5},
	std::vector<uint8_t>{5, 4, 9, 2, 3, 11},
	std::vector<uint8_t>{4, 8, 5, 8, 3, 5, 3, 1, 5},
	std::vector<uint8_t>{0, 5, 4, 1, 5, 0},
	std::vector<uint8_t>{5, 4, 9, 3, 0, 8},
	std::vector<uint8_t>{5, 4, 9},
	std::vector<uint8_t>{11, 4, 7, 11, 9, 4, 11, 10, 9},
	std::vector<uint8_t>{0, 3, 8, 11, 4, 7, 11, 9, 4, 11, 10, 9},
	std::vector<uint8_t>{11, 10, 7, 10, 1, 0, 7, 10, 0, 7, 0, 4},
	std::vector<uint8_t>{3, 10, 1, 11, 10, 3, 7, 8, 4},
	std::vector<uint8_t>{3, 2, 10, 3, 10, 4, 10, 9, 4, 7, 3, 4},
	std::vector<uint8_t>{9, 2, 10, 0, 2, 9, 8, 4, 7},
	std::vector<uint8_t>{3, 4, 7, 0, 4, 3, 1, 2, 10},
	std::vector<uint8_t>{7, 8, 4, 10, 1, 2},
	std::vector<uint8_t>{7, 11, 4, 4, 11, 9, 9, 11, 2, 9, 2, 1},
	std::vector<uint8_t>{1, 9, 0, 4, 7, 8, 2, 3, 11},
	std::vector<uint8_t>{7, 11, 4, 11, 2, 4, 2, 0, 4},
	std::vector<uint8_t>{4, 7, 8, 2, 3, 11},
	std::vector<uint8_t>{9, 4, 1, 4, 7, 1, 7, 3, 1},
	std::vector<uint8_t>{7, 8, 4, 1, 9, 0},
	std::vector<uint8_t>{3, 4, 7, 0, 4, 3},
	std::vector<uint8_t>{7, 8, 4},
	std::vector<uint8_t>{11, 10, 8, 8, 10, 9},
	std::vector<uint8_t>{0, 3, 9, 3, 11, 9, 11, 10, 9},
	std::vector<uint8_t>{1, 0, 10, 0, 8, 10, 8, 11, 10},
	std::vector<uint8_t>{10, 3, 11, 1, 3, 10},
	std::vector<uint8_t>{3, 2, 8, 2, 10, 8, 10, 9, 8},
	std::vector<uint8_t>{9, 2, 10, 0, 2, 9},
	std::vector<uint8_t>{8, 3, 0, 10, 1, 2},
	std::vector<uint8_t>{2, 10, 1},
	std::vector<uint8_t>{2, 1, 11, 1, 9, 11, 9, 8, 11},
	std::vector<uint8_t>{11, 2, 3, 9, 0, 1},
	std::vector<uint8_t>{11, 0, 8, 2, 0, 11},
	std::vector<uint8_t>{3, 11, 2},
	std::vector<uint8_t>{1, 8, 3, 9, 8, 1},
	std::vector<uint8_t>{1, 9, 0},
	std::vector<uint8_t>{8, 3, 0},
	std::vector<uint8_t>{}
};

//6 <-> 7;
//2 <-> 3;