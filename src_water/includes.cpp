#pragma once
#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <array>
#include <map>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <sstream>
#include <memory>
#include <chrono>
#include <set>
#include <unordered_set>
#include <deque>
#include <variant>
#include <functional>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define GLM_FORCE_SWIZZLE
#include "glm\glm.hpp"
#include "glm\gtx\matrix_transform_2d.hpp"
#include "glm\gtx\transform.hpp"

#include "PerlinNoise.hpp"