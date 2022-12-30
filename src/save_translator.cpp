#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <iostream>

struct Data_pair {
    uint16_t id;
    uint16_t count;
};

struct Vec2 {
    int x;
    int y;
};

struct Chunk {
    Vec2 loc;
    std::vector<Data_pair> data;
};

struct Byte_vec {
    std::vector<uint8_t> vec;

    void insert(uint8_t a) {
        vec.push_back(a);
    }

    void insert(uint16_t a) {
        vec.push_back(a & 255);
        vec.push_back(a >> 8);
    }

    void insert(int32_t a) {
        vec.push_back(a & 0xFF);
        vec.push_back((a & 0xFFFF) >> 8);
        vec.push_back((a & 0xFFFFFF) >> 16);
        vec.push_back(a >> 24);
    }
};

int main() {
    std::vector<Chunk> chunks;

    std::ifstream save;
    save.open("output\\save.bin", std::ios::in | std::ios::binary);

    if(save.is_open()) {
        uint16_t info_id;
        bool end = false;
        while (!end) {
            save.read((char*)&info_id, 2);

            if(info_id == (uint16_t)0xFFFF) {
                end = true;
            } else if(info_id == (uint16_t)0x0000) {
                chunks.push_back(Chunk());

                Chunk& chunk = chunks[chunks.size() - 1];

                save.read((char*)&chunk.loc.x, 4);
                save.read((char*)&chunk.loc.y, 4);

                int i = 0;

                while(i < 512) {
                    Data_pair d;

                    save.read((char*)&d.id, 2);
                    save.read((char*)&d.count, 2);

                    i += d.count;

                    chunk.data.push_back(d);
                }
            } else {
                end = true;
                std::cout << "input error" << std::endl;
            }
        }
        save.close();
    } else {
        std::cout << "unable to open save.txt" << std::endl;
    }

    std::ofstream output;
    output.open("output\\save1.bin", std::ios::out | std::ios::trunc | std::ios::binary);

    Byte_vec byte_vec;

    if(output.is_open()) {
        for(Chunk& chunk : chunks) {
            byte_vec.insert((uint16_t)0);
            byte_vec.insert(chunk.loc.x);
            byte_vec.insert(chunk.loc.y);

            for(Data_pair d : chunk.data) {
                byte_vec.insert((uint16_t)d.id);
                byte_vec.insert((uint16_t)d.count);
            }
        }
    } else {
        std::cout << "output error" << std::endl;
    }

    byte_vec.insert((uint16_t)0xFFFF);

    output.write((const char*)byte_vec.vec.data(), byte_vec.vec.size());

    output.close();
}