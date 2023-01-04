#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <vector>
#include <array>
#include <fstream>
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

int main() {
    std::string input_path, output_path;
    std::cin >> input_path >> output_path;

    std::vector<uint8_t> byte_vec;
    int x;
    int y;
    int c;

    uint8_t* data = stbi_load(input_path.data(), &x, &y, &c, 0);

    byte_vec.resize(14);

    byte_vec[0] = 'q';
    byte_vec[1] = 'o';
    byte_vec[2] = 'i';
    byte_vec[3] = 'f';
    
    byte_vec[4] = x >> 24;
    byte_vec[5] = x >> 16;
    byte_vec[6] = x >> 8;
    byte_vec[7] = x;

    byte_vec[8] = y >> 24;
    byte_vec[9] = y >> 16;
    byte_vec[10] = y >> 8;
    byte_vec[11] = y;

    byte_vec[12] = c;
    byte_vec[13] = 0;

    if(c == 3) {
        int data_len = x * y * c;
        int data_pos = 0;

        std::array<Color_rgb, 64> array;
        array.fill(Color_rgb{0, 0, 0});

        int prev_index = 64;
        Color_rgb prev_color = {0, 0, 0};

        int counter = 0;
        
        while(data_pos < data_len) {
            bool finish = false;
            Color_rgb color = {data[data_pos], data[data_pos + 1], data[data_pos + 2]};
            
            int array_index = (color.r * 3 + color.g * 5 + color.b * 7 + 0xFF * 11) % 64;

            if(prev_color == color) {
                prev_index = 64;
                counter++;
            } else {
                if(array[array_index] == color) {
                    if(array_index == prev_index) {
                        counter++;
                    } else {
                        if(counter != 0) {
                            byte_vec.push_back(0xC0 | (counter - 1));
                            counter = 0;
                        }

                        byte_vec.push_back(array_index);
                        prev_index = array_index;
                    }
                } else {
                    finish = false;
                    
                    int16_t rdif = color.r - prev_color.r;
                    int16_t gdif = color.g - prev_color.g;
                    int16_t bdif = color.b - prev_color.b;

                    int16_t gdif2 = gdif;
                    int16_t drdgdif = rdif - gdif;
                    int16_t dbdgdif = bdif - gdif;

                    if(rdif >= 254) rdif -= 256;
                    else if(rdif == -255) rdif = 1;

                    if(gdif >= 254) gdif -= 256;
                    else if(gdif == -255) gdif = 1;

                    if(bdif >= 254) bdif -= 256;
                    else if(bdif == -255) bdif = 1;

                    if(drdgdif >= 248) drdgdif -= 256;
                    else if(drdgdif <= -249) drdgdif += 256;

                    if(dbdgdif >= 248) dbdgdif -= 256;
                    else if(dbdgdif <= -249) dbdgdif += 256;

                    if(gdif2 >= 224) gdif2 -= 256;
                    else if(gdif2 <= -225) gdif2 += 256;

                    if(rdif >= -2 && rdif <= 1 && gdif >= -2 && gdif <= 1 && bdif >= -2 && bdif <= 1) {
                        if(counter != 0) {
                            byte_vec.push_back(0xC0 | (counter - 1));
                            counter = 0;
                        }
                        prev_index = 64;

                        byte_vec.push_back(0x40 | ((rdif + 2) << 4) | ((gdif + 2) << 2) | (bdif + 2));
                    } else if(gdif2 >= -32 && gdif2 <= 31 && drdgdif >= -8 && drdgdif <= 7 && dbdgdif >= -8 && dbdgdif <= 7) {
                        if(counter != 0) {
                            byte_vec.push_back(0xC0 | (counter - 1));
                            counter = 0;
                        }
                        prev_index = 64;
                        
                        byte_vec.push_back(0x80 | (gdif2 + 32));
                        byte_vec.push_back(((drdgdif + 8) << 4) | (dbdgdif + 8));
                    } else {
                        if(counter != 0) {
                            byte_vec.push_back(0xC0 | (counter - 1));
                            counter = 0;
                        }
                        prev_index = 64;

                        byte_vec.push_back(0xFE);
                        byte_vec.push_back(color.r);
                        byte_vec.push_back(color.g);
                        byte_vec.push_back(color.b);
                    }
                }
            }
            if(counter == 63) {
                byte_vec.push_back(0xC0 | (counter - 2));
                counter = 1;
            }

            array[array_index] = color;

            prev_color = color;

            data_pos += 3;
        }
        if(counter != 0) {
            byte_vec.push_back(0xC0 | (counter - 1));
            counter = 0;
        }
    } else if(c == 4) {
        int data_len = x * y * c;
        int data_pos = 0;

        std::array<Color_rgba, 64> array;
        array.fill(Color_rgba{0, 0, 0, 0});

        int prev_index = 64;
        Color_rgba prev_color = {0, 0, 0, 0xFF};

        int counter = 0;
        
        while(data_pos < data_len) {
            bool finish = false;
            Color_rgba color = {data[data_pos], data[data_pos + 1], data[data_pos + 2], data[data_pos + 3]};
            
            int array_index = (color.r * 3 + color.g * 5 + color.b * 7 + color.a * 11) % 64;

            if(prev_color == color) {
                prev_index = 64;
                counter++;
            } else {
                if(array[array_index] == color) {
                    if(array_index == prev_index) {
                        counter++;
                    } else {
                        if(counter != 0) {
                            byte_vec.push_back(0xC0 | (counter - 1));
                            counter = 0;
                        }

                        byte_vec.push_back(array_index);
                        prev_index = array_index;
                    }
                } else {
                    finish = false;

                    int16_t adif = color.a - prev_color.a;
                    
                    if(adif == 0) {
                        int16_t rdif = color.r - prev_color.r;
                        int16_t gdif = color.g - prev_color.g;
                        int16_t bdif = color.b - prev_color.b;

                        int16_t gdif2 = gdif;
                        int16_t drdgdif = rdif - gdif;
                        int16_t dbdgdif = bdif - gdif;

                        if(rdif >= 254) rdif -= 256;
                        else if(rdif == -255) rdif = 1;

                        if(gdif >= 254) gdif -= 256;
                        else if(gdif == -255) gdif = 1;

                        if(bdif >= 254) bdif -= 256;
                        else if(bdif == -255) bdif = 1;

                        if(drdgdif >= 248) drdgdif -= 256;
                        else if(drdgdif <= -249) drdgdif += 256;

                        if(dbdgdif >= 248) dbdgdif -= 256;
                        else if(dbdgdif <= -249) dbdgdif += 256;

                        if(gdif2 >= 224) gdif2 -= 256;
                        else if(gdif2 <= -225) gdif2 += 256;

                        if(rdif >= -2 && rdif <= 1 && gdif >= -2 && gdif <= 1 && bdif >= -2 && bdif <= 1) {
                            if(counter != 0) {
                                byte_vec.push_back(0xC0 | (counter - 1));
                                counter = 0;
                            }
                            prev_index = 64;

                            byte_vec.push_back(0x40 | ((rdif + 2) << 4) | ((gdif + 2) << 2) | (bdif + 2));

                            finish = true;
                        } else if(gdif2 >= -32 && gdif2 <= 31 && drdgdif >= -8 && drdgdif <= 7 && dbdgdif >= -8 && dbdgdif <= 7) {
                            if(counter != 0) {
                                byte_vec.push_back(0xC0 | (counter - 1));
                                counter = 0;
                            }
                            prev_index = 64;
                            
                            byte_vec.push_back(0x80 | (gdif2 + 32));
                            byte_vec.push_back(((drdgdif + 8) << 4) | (dbdgdif + 8));

                            finish = true;
                        }
                    }
                    if(!finish) {
                        if(counter != 0) {
                            byte_vec.push_back(0xC0 | (counter - 1));
                            counter = 0;
                        }
                        prev_index = 64;

                        if(adif == 0) {
                            byte_vec.push_back(0xFE);
                            byte_vec.push_back(color.r);
                            byte_vec.push_back(color.g);
                            byte_vec.push_back(color.b);
                        } else {
                            byte_vec.push_back(0xFF);
                            byte_vec.push_back(color.r);
                            byte_vec.push_back(color.g);
                            byte_vec.push_back(color.b);
                            byte_vec.push_back(color.a);   
                        }
                    }
                }
            }
            if(counter == 63) {
                byte_vec.push_back(0xC0 | (counter - 2));
                counter = 1;
            }

            array[array_index] = color;

            prev_color = color;

            data_pos += 4;
        }
        if(counter != 0) {
            byte_vec.push_back(0xC0 | (counter - 1));
            counter = 0;
        }
    }

    byte_vec.push_back(0x00);
    byte_vec.push_back(0x00);
    byte_vec.push_back(0x00);
    byte_vec.push_back(0x00);
    byte_vec.push_back(0x00);
    byte_vec.push_back(0x00);
    byte_vec.push_back(0x00);
    byte_vec.push_back(0x01);

    std::ofstream qoi_file;
    qoi_file.open(output_path.data(), std::ios::out | std::ios::trunc | std::ios::binary);

    qoi_file.write((const char*)byte_vec.data(), byte_vec.size());

    qoi_file.close();
}