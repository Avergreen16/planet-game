#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

std::vector<uint8_t> get_data_from_file(char* path) {
    std::ifstream file;
    file.open(path);

    file.seekg(0, std::ios::end);
    int file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << file_size << std::endl;

    std::vector<uint8_t> data(file_size);
    file.read((char*)data.data(), file_size);

    file.close();

    return data;
}

int main() {
    std::string filepath_a, filepath_b;
    std::cin >> filepath_a >> filepath_b;

    std::vector<uint8_t> file_a = get_data_from_file(filepath_a.data());
    std::vector<uint8_t> file_b = get_data_from_file(filepath_b.data());

    if(file_a.size() != file_b.size()) {
        std::cout << "files are not equal :( -> file size: " << file_a.size() << " vs " << file_b.size() << std::endl;
        
        int file_len = std::min(file_a.size(), file_b.size());
        int pos = -1;
        for(int i = 0; i < file_len; i++) {
            if(file_a[i] != file_b[i]) {
                pos = i;
                break;
            }
        }

        std::cout << "file difference starts at ";
        if(pos == -1) {
            std::cout << file_len << std::endl;
        } else {
            std::cout << pos << std::endl;
        }
    } else {
        int file_len = file_a.size();
        for(int i = 0; i < file_len; i++) {
            if(file_a[i] != file_b[i]) {
                std::cout << "files are not equal :( -> pos: " << std::ios_base::hex << i << std::endl;
                return 0;
            }
        }

        std::cout << "files are equal! :)" << std::endl;
    }
}