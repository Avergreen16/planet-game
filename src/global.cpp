#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <array>
#include <unordered_map>
#include <queue>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "PerlinNoise.hpp"
#include "glm\glm.hpp"
#include "glm\gtx\transform.hpp"