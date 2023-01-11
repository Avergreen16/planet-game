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
#include <cstring>

struct Node {
    int freq;
    uint8_t data;
    Node* left = NULL;
    Node* right = NULL;
    int code_length = 0;
};

struct Huff_length {
    uint8_t byte;
    int length;
};

struct node_sort {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

struct huffman_sort {
    bool operator()(Huff_length& a, Huff_length& b) {
        return (a.length < b.length) || (a.byte < b.byte);
    }
};

void increment(std::array<uint8_t, 8>& bytes) {
    for(int i = 7; i >= 0; i--) {
        uint8_t& b = bytes[i];
        b++;
        if(b != 0) {
            break;
        }
    }
}

void insert_into_bytes(std::vector<uint8_t>& bytes, uint8_t& bit_pos, unsigned int in, uint8_t size) {
    int pos_in = size - 1;

    int last_index = bytes.size() - 1;

    while(pos_in >= 0) {
        int shift_byte = pos_in - 7;
        
        uint8_t insert_byte = 0;
        if(shift_byte >= 0) {
            insert_byte = in >> shift_byte;
        } else {
            insert_byte = in << -shift_byte;
        }

        if(bit_pos == 7) {
            bytes.push_back(insert_byte);
            last_index++;
        } else {
            bytes[last_index] |= (insert_byte >> (7 - bit_pos));
            if(pos_in > bit_pos) {
                bytes.push_back(0);
                last_index++;

                bytes[last_index] |= (insert_byte << (bit_pos + 1));
            }
        }
        pos_in -= 8;
    }

    bit_pos = (bit_pos - size) % 8u;
}

void insert_into_bytes(std::vector<uint8_t>& bytes, uint8_t& bit_pos, uint64_t in, uint8_t size) {
    int pos_in = size - 1;

    int last_index = bytes.size() - 1;

    while(pos_in >= 0) {
        int shift_byte = pos_in - 7;
        
        uint8_t insert_byte = 0;
        if(shift_byte >= 0) {
            insert_byte = in >> shift_byte;
        } else {
            insert_byte = in << -shift_byte;
        }

        if(bit_pos == 7) {
            bytes.push_back(insert_byte);
            last_index++;
        } else {
            bytes[last_index] |= (insert_byte >> (7 - bit_pos));
            if(pos_in > bit_pos) {
                bytes.push_back(0);
                last_index++;

                bytes[last_index] |= (insert_byte << (bit_pos + 1));
            }
        }
        pos_in -= 8;
    }

    bit_pos = (bit_pos - size) % 8u;
}

unsigned int retrieve_bits(std::vector<uint8_t>& bytes, unsigned int& byte_pos, uint8_t& bit_pos, unsigned int size) {
    int result_pos = size - 1;
    unsigned int result = 0;
    int current_bit_pos = bit_pos;

    while(result_pos >= 0) {
        uint8_t byte = bytes[byte_pos];
        int shift = result_pos - current_bit_pos;

        if(shift >= 0) {
            result |= int(byte) << shift;
        } else {
            result |= int(byte) >> -shift;
        }
        result_pos -= current_bit_pos + 1;
        current_bit_pos = 7;
        byte_pos++;
    }

    byte_pos--;

    bit_pos = (bit_pos - size) % 8u;

    int clear_shift = (sizeof(result) * 8 - size);
    return (result << clear_shift) >> clear_shift;
}

void shift(std::array<uint8_t, 8>& a) {
    bool prev_carry = false;
    bool carry = false;
    for(int i = 7; i >= 0; i--) {
        if((a[i] >> 7) == 1) {
            carry = true;
        } else {
            carry = false;
        }
        a[i] <<= 1;
        if(prev_carry) a[i] |= 1;
        prev_carry = carry;
    }
}

std::vector<uint8_t> get_data_from_file(char* path) {
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::ate | std::ios::binary);

    if(file.is_open()) {
        int file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(file_size);
        file.read((char*)data.data(), file_size);

        file.close();

        return data;
    } else {
        std::cout << "file error" << std::endl;
    }
}

int search_sliding_buffer(std::deque<uint8_t>& sliding_buffer, int look_ahead_length, int search_length) {
    int look_ahead_0 = sliding_buffer.size() - look_ahead_length;
    for(int i = 0; i < look_ahead_0; i++) {
        if(memcmp(&sliding_buffer[i], &sliding_buffer[look_ahead_0], search_length) == 0) {
            return -(i - look_ahead_0);
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

struct Decode_node {
    uint8_t value;
    Decode_node* left = 0;
    Decode_node* right = 0;
};

int main() {
    /*std::vector<uint8_t> data;

    unsigned int counter = 0;
    uint8_t bit_pos = 7;
    for(int i = 0; i < 4001; i++) {
        unsigned int value = rand();
        uint8_t length = (rand() % 2) ? 9 : 21;

        counter += length;
        insert_into_bytes(data, bit_pos, value, length);
        std::cout << (int)bit_pos << " ";
    }

    std::cout << (int)ceil(double(counter) / 8) << " " << data.size() << std::endl;*/

    std::vector<uint8_t> data = get_data_from_file("saves\\a.txt");

    /*std::vector<Data> compressed_data;
    std::deque<uint8_t> sliding_buffer;
    
    int search_buffer_size = 256;
    int look_ahead_buffer_size = std::min(4096, int(data.size()));
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
            compressed_data.push_back(Data({found_loc - 1, length - 1}));
            buffer_move = length;
        }

        for(int i = 0; i < buffer_move; i++) {
            data_pos++;
            if(data_pos + 255 < data.size()) {
                sliding_buffer.push_back(data[data_pos + 255]);
            } else {
                look_ahead_buffer_size--;
            }
            if(sliding_buffer.size() - look_ahead_buffer_size >= search_buffer_size) {
                sliding_buffer.pop_front();
            }
        }
    }

    std::vector<uint8_t> lzss_data;

    int data_size = 0;

    unsigned int byte_pos = 0;
    uint8_t bit_pos = 7;

    for(Data d : compressed_data) {
        if(d.type == 1) {
            std::array<int, 2> pointer = std::any_cast<std::array<int, 2>>(d.contents);
            unsigned int pointer_int = 0x100000 | (pointer[0] << 8) | pointer[1];

            insert_into_bytes(lzss_data, bit_pos, pointer_int, 21);
            data_size += 21;

        } else {
            uint8_t c = std::any_cast<uint8_t>(d.contents);
            
            insert_into_bytes(lzss_data, bit_pos, (unsigned int)c, 9);

            data_size += 9;
        }
    }
    
    std::cout << (int)ceil(double(data_size) / 8) << " " << lzss_data.size() << "\n" << std::hex;*/
    
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

    std::vector<Huff_length> huffman_lengths;

    std::queue<Node*> traverse_queue;
    traverse_queue.push(priority_queue.top());

    while(traverse_queue.size() != 0) {
        Node* n = traverse_queue.front();
        traverse_queue.pop();

        if(n->left == NULL) {
            huffman_lengths.push_back(Huff_length{n->data, n->code_length});
        } else {
            n->left->code_length = n->code_length + 1;
            n->right->code_length = n->code_length + 1;
            traverse_queue.push(n->left);
            traverse_queue.push(n->right);
        }
    }

    std::cout << "\n";

    std::sort(huffman_lengths.begin(), huffman_lengths.end(), 
    [](const Huff_length& a, const Huff_length& b) {
        return (a.length < b.length) || (a.length == b.length && a.byte < b.byte);
    }
    );

    //std::vector<std::tuple<uint8_t, std::string, uint64_t, uint8_t>> code_map;
    std::map<uint8_t, std::pair<uint64_t, uint8_t>> code_map;

    int len = huffman_lengths[0].length;
    uint64_t code = 0;
    for(Huff_length c : huffman_lengths) {
        if(len != c.length) {
            code <<= 1;
            len = c.length;
        }
        code_map.insert({c.byte, {code, c.length}});
        code++;
    }

    std::vector<uint8_t> final_data;
    
    unsigned int byte_pos = 2;
    uint8_t bit_pos = 7;

    insert_into_bytes(final_data, bit_pos, (unsigned int)code_map.size(), 16);
    for(Huff_length l : huffman_lengths) {
        insert_into_bytes(final_data, bit_pos, (unsigned int)l.byte, 8);
        insert_into_bytes(final_data, bit_pos, (unsigned int)l.length, 8);
        byte_pos += 2;
    }

    int final_bits_byte = byte_pos;
    insert_into_bytes(final_data, bit_pos, (unsigned int)0, 8);
    insert_into_bytes(final_data, bit_pos, (unsigned int)0, 32);
    int beg_size = final_data.size();

    for(uint8_t byte : data) {
        std::pair<uint64_t, uint8_t> pair = code_map[byte];
        insert_into_bytes(final_data, bit_pos, pair.first, pair.second);
    }

    unsigned int body_size = final_data.size() - beg_size;
    final_data[final_bits_byte] = (bit_pos + 1) % 8u;
    memcpy(&final_data[final_bits_byte + 1], &body_size, 4);

    std::cout << std::hex << "previous size: " << data.size() << " bytes\nnew size: " << final_data.size() << " bytes\n";

    // decode;

    std::list<Decode_node> decode_nodes;
    Decode_node root;

    byte_pos = 2;
    bit_pos = 7;

    uint16_t data_count = (final_data[0] << 8) | final_data[1];

    uint8_t prev_length = 0;
    uint64_t huff_code = 0;
    for(uint16_t i = 0; i < data_count; i++) {
        uint8_t byte = final_data[byte_pos];
        uint8_t length = final_data[byte_pos + 1];

        huff_code <<= (length - prev_length);
        prev_length = length;

        std::string code;

        Decode_node* current_node = &root;
        for(int i = length - 1; i >= 0; i--) {
            if((huff_code >> i) & 1) {
                code += '1';
                if(current_node->right == 0) {
                    decode_nodes.push_back(Decode_node());
                    current_node->right = &decode_nodes.back();
                }
                current_node = current_node->right;
            } else {
                code += '0';
                if(current_node->left == 0) {
                    decode_nodes.push_back(Decode_node());
                    current_node->left = &decode_nodes.back();
                }
                current_node = current_node->left;
            }
        }
        std::cout << (uint16_t)byte << " " << code << "\n";

        current_node->value = byte;

        byte_pos += 2;
        huff_code++;
    }
    std::cout << byte_pos << " " << final_bits_byte << "\n";
    uint8_t final_bits = final_data[byte_pos];
    unsigned int body_length = *(unsigned int*)&final_data[byte_pos + 1];
    byte_pos += 5;
    int body_start_pos = byte_pos;

    std::cout << decode_nodes.size() << " " << body_length << "\n";

    std::vector<uint8_t> inflated_data;

    Decode_node* current_node = &root;
    while(!(byte_pos >= body_start_pos + body_size - 1 && bit_pos < final_bits)) {
        uint8_t bit = ((final_data[byte_pos] >> bit_pos) & 1);

        if(bit) {
            current_node = current_node->right;
        } else {
            current_node = current_node->left;
        }

        if(current_node->right == 0) {
            inflated_data.push_back(current_node->value);
            current_node = &root;
        }

        if(bit_pos == 0) {
            byte_pos++;
            bit_pos = 7;
        } else {
            bit_pos--;
        }
    }

    std::cout << "inflated data size: " << inflated_data.size() << "\n";
    
    inflated_data.push_back('\0');
    std::cout << (char*)inflated_data.data() << "\n";
}