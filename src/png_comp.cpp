#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <vector>
#include <array>
#include <fstream>
#include <iostream>

int main() {
    std::string filepath_a, filepath_b;
    std::cin >> filepath_a >> filepath_b;
    
    int x_a, y_a, c_a, x_b, y_b, c_b;

    uint8_t* data_a = stbi_load(filepath_a.data(), &x_a, &y_a, &c_a, 0);
    uint8_t* data_b = stbi_load(filepath_a.data(), &x_b, &y_b, &c_b, 0);

    if(!(x_a == x_b && y_a == y_b)) {
        std::cout << "images not the same (different sizes)" << std::endl;
    } else if(!(c_a == c_b)) {
        std::cout << "images not the same (different color channels)" << std::endl;
    } else {
        int length = x_a * y_a * c_a;
        for(int i = 0; i < length; i++) {
            if(data_a[i] != data_b[i]) {
                std::cout << "images are not the same (pixel difference) location: " << ((i / c_a) % y_a) << " " << ((i / c_a) / y_a) << std::endl;
                return 0;
            }
        }
        std::cout << "images are the same :)" << std::endl;
    }
}