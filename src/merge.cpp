#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "glm\glm.hpp"

#include <cstdint>
#include <array>

int main() {
    std::array<int, 3> lr_attr;
    std::array<int, 3> tb_attr;
    uint8_t* lr_data = stbi_load("res\\leftright.png", &lr_attr[0], &lr_attr[1], &lr_attr[2], 0);
    uint8_t* tb_data = stbi_load("res\\top.png", &tb_attr[0], &tb_attr[1], &tb_attr[2], 0);

    int num_pixels = lr_attr[0] * lr_attr[1];
    uint8_t* new_data = new uint8_t[num_pixels * 4];
    
    for(int i = 0; i < num_pixels * 4; i += 4) {
        glm::vec4 lr_vec = {float(lr_data[i]) / 128 - 1.0, float(lr_data[i + 1]) / 128 - 1.0, float(lr_data[i + 2]) / 128 - 1.0, float(lr_data[i + 3]) / 256};
        glm::vec4 tb_vec = {float(tb_data[i]) / 128 - 1.0, float(tb_data[i + 1]) / 128 - 1.0, float(tb_data[i + 2]) / 128 - 1.0, float(tb_data[i + 3]) / 256};

        if(lr_vec.a != 0.0) {
            glm::vec2 norm_lr = normalize(glm::vec2(lr_vec.r, lr_vec.g));
            glm::vec2 norm_tb = normalize(glm::vec2(tb_vec.g, tb_vec.b));

            glm::vec3 new_dir = normalize(glm::vec3(norm_lr.x * abs(norm_tb.x), norm_lr.y * abs(norm_tb.x), norm_tb.y));

            new_data[i] = std::min(new_dir.x * 128 + 128, 255.0f);
            new_data[i + 1] = std::min(new_dir.y * 128 + 128, 255.0f);
            new_data[i + 2] = std::min(new_dir.z * 128 + 128, 255.0f);
            new_data[i + 3] = 255;
        } else {
            new_data[i] = 0;
            new_data[i + 1] = 0;
            new_data[i + 2] = 0;
            new_data[i + 3] = 0;
        }
    }
    
    stbi_write_png("output\\normalmap.png", lr_attr[0], lr_attr[1], lr_attr[2], new_data, lr_attr[0] * lr_attr[2]);

    delete[] new_data;
}