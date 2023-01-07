#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <queue>
#include <list>
#include <sstream>
#include <cmath>
#include <any>
#include <array>
#include <algorithm>

struct Node {
    int freq;
    uint8_t data;
    Node* left = NULL;
    Node* right = NULL;
    int code_length;
};

struct Huffman_code {
    uint8_t byte;
    int length;
    std::string code;
};

struct node_sort {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

struct huffman_sort {
    bool operator()(Huffman_code& a, Huffman_code& b) {
        return (a.length < b.length) || (a.byte < b.byte);
    }
};

std::vector<uint8_t> get_data_from_file(char* path) {
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::ate | std::ios::binary);

    int file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(file_size);
    file.read((char*)data.data(), file_size);

    file.close();

    return data;
}

int search_sliding_buffer(std::deque<uint8_t>& sliding_buffer, int look_ahead_length, int search_length) {
    int look_ahead_0 = sliding_buffer.size() - look_ahead_length;
    for(int i = 0; i < look_ahead_0; i++) {
        if(sliding_buffer[i] == sliding_buffer[look_ahead_0]) {
            bool match_found = true;
            for(int j = 1; j < search_length; j++) {
                if(sliding_buffer[i + j] != sliding_buffer[look_ahead_0 + j]) {
                    match_found = false;
                    break;
                }
            }
            if(match_found) {
                return -(i - look_ahead_0);
            }
        }
    }

    return -1;
}

struct Data {
    int type = 0;
    std::any contents;

    Data(uint8_t byte) {
        contents = byte;
    }

    Data(std::array<int, 2> pointer) {
        contents = pointer;
        type = 1;
    }
};

int main() {
    std::vector<uint8_t> data = get_data_from_file("saves\\a.txt");
    /*std::vector<Data> compressed_data;
    std::deque<uint8_t> sliding_buffer;
    
    int search_buffer_size = 4096;
    int look_ahead_buffer_size = std::min(256, int(data.size()));
    int data_pos = 0;

    for(int i = 0; i < look_ahead_buffer_size; i++) {
        sliding_buffer.push_back(data[i]);
    }

    while(data_pos < data.size()) {
        int buffer_move = 1;

        int found_ended = false;
        int length = 0;
        int found_loc = -1;

        while(length < look_ahead_buffer_size) {
            length++;
            int found_result = search_sliding_buffer(sliding_buffer, look_ahead_buffer_size, length);
            if(found_result == -1) {
                found_ended = true;
                break;
            } else {
                found_loc = found_result;
            }
        }
        if(found_loc == -1 || length < 3) {
            compressed_data.push_back(Data(data[data_pos]));
            buffer_move = 1;
        } else {
            if(found_ended) {
                length--;
            }
            compressed_data.push_back(Data({found_loc, length}));
            buffer_move = length;
        }

        for(int i = 0; i < buffer_move; i++) {
            data_pos++;
            if(data_pos + 255 < data.size()) {
                sliding_buffer.push_back(data[data_pos + 255]);
            } else {
                look_ahead_buffer_size--;
            }
            if(sliding_buffer.size() - look_ahead_buffer_size >= 4096) {
                sliding_buffer.pop_front();
            }
        }
    }

    std::string decoded_string;

    int bits = 0;

    for(Data d : compressed_data) {
        if(d.type == 1) {
            std::array<int, 2> pointer = std::any_cast<std::array<int, 2>>(d.contents);
            std::cout << "pointer: " << pointer[0] << " " << pointer[1] << "\n";

            for(int i = pointer[0]; i < pointer[0] + pointer[1]; i++) {
                decoded_string += decoded_string[data.size() - i];
            }

            bits += 21;
        } else {
            char c = std::any_cast<uint8_t>(d.contents);
            std::cout << c << "\n";
            bits += 9;

            decoded_string += c;
        }
    }

    std::cout << decoded_string << "\n";

    std::cout << "original size: " << data.size() << " bytes\ncompressed_size: " << (int)ceil(double(bits) / 8) << " bytes\n";*/

    

    std::map<uint8_t, int> frequencies;

    for(uint8_t b : data) {
        if(frequencies.contains(b)) {
            frequencies[b]++;
        } else {
            frequencies.insert({b, 1});
        }
    }

    std::list<Node> nodes;
    for(auto [val, freq] : frequencies) {
        nodes.push_back(Node{freq, val});
    }

    std::priority_queue<Node*, std::vector<Node*>, node_sort> priority_queue;
    for(Node& n : nodes) {
        priority_queue.push(&n);
    }

    while(priority_queue.size() != 1) {
        Node* node0 = priority_queue.top();
        priority_queue.pop();
        Node* node1 = priority_queue.top();
        priority_queue.pop();

        nodes.push_back(Node{node0->freq + node1->freq, 0x0, node0, node1});
        priority_queue.push(&nodes.back());
    }

    std::vector<Huffman_code> huffman_codes;

    std::queue<Node*> traverse_queue;
    traverse_queue.push(priority_queue.top());

    while(traverse_queue.size() != 0) {
        Node* n = traverse_queue.front();
        traverse_queue.pop();

        if(n->left == NULL) {
            huffman_codes.push_back(Huffman_code{n->data, n->code_length});
        } else {
            n->left->code_length++;
            n->right->code_length++;
            traverse_queue.push(n->left);
            traverse_queue.push(n->right);
        }
    }

    std::sort(huffman_codes.begin(), huffman_codes.end(), 
    [](Huffman_code& a, Huffman_code& b) {
        (a.length < b.length) || (a.byte < b.byte);
    }
    );

    std::cout << huffman_codes.size() << "\n";

    std::map<uint8_t, std::string> code_map;
    int len = 1;
    int starting_value = 0;
    for(Huffman_code c : huffman_codes) {
        while(floor(log(c.length)) + 1 < len) {
            starting_value << 1;
            
        }
    }

    int collective_size = 0;
    for(auto [data, code] : code_map) {
        std::cout << (int)data << " " << code << std::endl;
        collective_size += frequencies[data] * code.length();
    }

    std::cout << "previous size: " << data.size() << " bytes\nnew size: " << int(ceil(float(collective_size) / 8)) << " bytes\n";
}