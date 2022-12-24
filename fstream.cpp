
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

struct Entity {
    Entity() = default;
    Entity(char* in_name, int in_hp, double in_speed) {
        strcpy(name, in_name);
        hit_points = in_hp;
        speed = in_speed;
    }

    int hit_points;
    double speed;
    char name[20];
};

int main() {
    std::ifstream my_file("file.txt", std::ios_base::in);
    char name[20];
    int hp;
    double speed;

    if(my_file.is_open()) {
        my_file.getline(name, 20);
        my_file >> hp >> speed;
    }

    Entity entity((char*)name, hp, speed);
    Entity copied_entity;

    std::ofstream out_stream("entity.bin", std::ios_base::out | std::ios_base::binary);

    if(out_stream.is_open()) {
        std::cout << "writing contents of entity to binary file\n";
        out_stream.write((const char*)&entity, sizeof(Entity));
        out_stream.close();
    }

    std::ifstream in_stream("entity.bin", std::ios_base::in | std::ios_base::binary);

    if(in_stream.is_open()) {
        std::cout << "retrieving data from binary file\n";
        in_stream.read((char*)&copied_entity, sizeof(Entity));
        in_stream.close();
    }

    std::cout << "entity attributes:\nhit_points -> " << copied_entity.hit_points << "\nspeed -> " << copied_entity.speed << "\nname -> " << copied_entity.name << std::endl;
}