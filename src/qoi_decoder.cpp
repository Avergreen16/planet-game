#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>

struct Color_rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Color_rgba {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

bool operator==(Color_rgba& a, Color_rgba& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

bool operator==(Color_rgb& a, Color_rgb& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

std::vector<uint8_t> get_data_from_file(char* path) {
    std::ifstream file;
    file.open(path);

    file.seekg(0, std::ios::end);
    int file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(file_size);
    file.read((char*)data.data(), file_size);

    file.close();

    return data;
}

int main() {
    std::string input_path, output_path;
    std::cin >> input_path >> output_path;

    std::vector<uint8_t> file = get_data_from_file(input_path.data());
    uint8_t* qoi_data = (uint8_t*)file.data();
    int length = file.size();
    int pos = 14;

    int x = ((int)qoi_data[4] << 24) | ((int)qoi_data[5] << 16) | ((int)qoi_data[6] << 8) | (int)qoi_data[7];
    int y = ((int)qoi_data[8] << 24) | ((int)qoi_data[9] << 16) | ((int)qoi_data[10] << 8) | (int)qoi_data[11];
    int c = qoi_data[12];

    std::vector<uint8_t> pixels(x * y * c);
    int pixels_pos = 0;

    if(c == 3) {
        Color_rgb prev_color = {0, 0, 0};

        std::array<Color_rgb, 64> array;
        array.fill({0, 0, 0});

        while(pos < length - 8) {
            uint8_t tag = qoi_data[pos];

            Color_rgb color;

            if(tag == 0xFE) {
                color = {qoi_data[pos + 1], qoi_data[pos + 2], qoi_data[pos + 3]};
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pos += 4;

                prev_color = color;
                array[(color.r * 3 + color.g * 5 + color.b * 7 + 0xFF * 11) % 64] = color;

                pixels_pos += 3;
            } else if((tag & 0xC0) == 0x00) {
                color = array[tag & 0x3F];
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pos += 1;

                prev_color = color;
                pixels_pos += 3;
            } else if((tag & 0xC0) == 0x40) {
                color = {uint8_t(prev_color.r + ((tag & 0x30) >> 4) - 2), uint8_t(prev_color.g + ((tag & 0x0C) >> 2) - 2), uint8_t(prev_color.b + (tag & 0x03) - 2)};
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pos += 1;

                prev_color = color;
                array[(color.r * 3 + color.g * 5 + color.b * 7 + 0xFF * 11) % 64] = color;

                pixels_pos += 3;
            } else if((tag & 0xC0) == 0x80) {
                uint8_t rb = qoi_data[pos + 1];
                uint8_t gdif = tag & 0x3F;

                color = {uint8_t(prev_color.r + ((rb >> 4) + gdif) - 40), uint8_t(prev_color.g + gdif - 32), uint8_t(prev_color.b + (int(rb & 0x0F) + gdif) - 40)};
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pos += 2;

                prev_color = color;
                array[(color.r * 3 + color.g * 5 + color.b * 7 + 0xFF * 11) % 64] = color;

                pixels_pos += 3;
            } else if((tag & 0xC0) == 0xC0) {
                uint8_t run_length = (tag & 0x3F) + 1;
                for(uint8_t i = 0; i < run_length; i++) {
                    pixels[pixels_pos] = prev_color.r;
                    pixels[pixels_pos + 1] = prev_color.g;
                    pixels[pixels_pos + 2] = prev_color.b;
                    pixels_pos += 3;
                }
                pos += 1;
            }
        }
    } else if(c == 4) {
        Color_rgba prev_color = {0, 0, 0, 0xFF};

        std::array<Color_rgba, 64> array;
        array.fill({0, 0, 0, 0});

        while(pos < length - 8) {
            uint8_t tag = qoi_data[pos];

            Color_rgba color;

            if(tag == 0xFF) {
                color = {qoi_data[pos + 1], qoi_data[pos + 2], qoi_data[pos + 3], qoi_data[pos + 4]};
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pixels[pixels_pos + 3] = color.a;
                pos += 5;

                prev_color = color;
                array[(color.r * 3 + color.g * 5 + color.b * 7 + color.a * 11) % 64] = color;

                pixels_pos += 4;
            } else if(tag == 0xFE) {
                color = {qoi_data[pos + 1], qoi_data[pos + 2], qoi_data[pos + 3], prev_color.a};
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pixels[pixels_pos + 3] = color.a;
                pos += 4;

                prev_color = color;
                array[(color.r * 3 + color.g * 5 + color.b * 7 + color.a * 11) % 64] = color;

                pixels_pos += 4;
            } else if((tag & 0xC0) == 0x00) {
                color = array[tag & 0x3F];
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pixels[pixels_pos + 3] = color.a;
                pos += 1;

                prev_color = color;
                pixels_pos += 4;
            } else if((tag & 0xC0) == 0x40) {
                color = {uint8_t(prev_color.r + ((tag & 0x30) >> 4) - 2), uint8_t(prev_color.g + ((tag & 0x0C) >> 2) - 2), uint8_t(prev_color.b + (tag & 0x03) - 2), prev_color.a};
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pixels[pixels_pos + 3] = color.a;
                pos += 1;

                prev_color = color;
                array[(color.r * 3 + color.g * 5 + color.b * 7 + color.a * 11) % 64] = color;

                pixels_pos += 4;
            } else if((tag & 0xC0) == 0x80) {
                uint8_t rb = qoi_data[pos + 1];
                uint8_t gdif = tag & 0x3F;

                color = {uint8_t(prev_color.r + ((rb >> 4) + gdif) - 40), uint8_t(prev_color.g + gdif - 32), uint8_t(prev_color.b + (int(rb & 0x0F) + gdif) - 40), prev_color.a};
                pixels[pixels_pos] = color.r;
                pixels[pixels_pos + 1] = color.g;
                pixels[pixels_pos + 2] = color.b;
                pixels[pixels_pos + 3] = color.a;
                pos += 2;

                prev_color = color;
                array[(color.r * 3 + color.g * 5 + color.b * 7 + color.a * 11) % 64] = color;

                pixels_pos += 4;
            } else if((tag & 0xC0) == 0xC0) {
                uint8_t run_length = (tag & 0x3F) + 1;
                for(uint8_t i = 0; i < run_length; i++) {
                    pixels[pixels_pos] = prev_color.r;
                    pixels[pixels_pos + 1] = prev_color.g;
                    pixels[pixels_pos + 2] = prev_color.b;
                    pixels[pixels_pos + 3] = prev_color.a;
                    pixels_pos += 4;
                }
                pos += 1;
            }
        }
    }

    std::ofstream output;
    output.open(output_path.data(), std::ios::out | std::ios::trunc | std::ios::binary);

    output.write((const char*)pixels.data(), pixels.size());

    output.close();
    //stbi_write_png(, x, y, c, pixels.data(), x * c);
}

