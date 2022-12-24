#pragma once
#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <array>
#include <map>
#include <queue>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "PerlinNoise.hpp"
#include "glm\glm.hpp"

#ifdef USE_2D
#include "glm\gtx\matrix_transform_2d.hpp"
#endif

#ifndef USE_2D
#include "glm\gtx\transform.hpp"
#endif