#include <iostream>
#include <utility>
#include <stdint.h>
#include <array>
#include <chrono>
#include <map>

time_t get_time() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

std::ostream& operator<<(std::ostream& os, std::array<short, 2> arr) {
    os << arr[0] << ", " << arr[1];
    return os;
}

template<typename type>
struct Dynamic_array {
    type* data;
    int size;

    Dynamic_array(int _size) {
        data = new type[_size];
        size = _size;
    }

    void resize(int _size) {
        type* new_data = new type[_size];
        memcpy(new_data, data, std::min(size, _size) * sizeof(type));
        size = _size;

        delete[] data;
        data = new_data;
    }

    type& operator[](int key) {
        return data[key];
    }

    ~Dynamic_array() {
        delete[] data;
    }
};

template<typename type>
struct Node {
    type data;
    Node* next;

    Node(type data) {
        this->data = data;
        next = 0;
    }
};

template<typename type>
struct Linked_list {
    Node<type>* front = 0x0;
    int size = 0;

    void push_front(type data) {
        Node<type>* temp = new Node(data);
        temp->next = front;
        front = temp;
        
        size++;
    }

    void push_front(type* data, int length) {
        for(int i = length - 1; i >= 0; i++) {
            Node<type>* temp = new Node(data[i]);
            temp->next = front;
            front = temp;

            size++;
        }
    }

    void remove(int key) {
        Node<type>* ptr = front;
        for(int i = 0; i < key - 1; i++) {
            ptr = ptr->next;
        }

        Node<type>* temp = ptr->next->next;
        delete ptr->next;
        ptr->next = temp;

        size--;
    }

    void insert(int key, type data) {
        Node<type>* ptr = front;
        for(int i = 0; i < key - 1; i++) {
            ptr = ptr->next;
        }

        Node<type>* temp = ptr->next;
        Node<type>* _new = new Node(data);
        _new->next = temp;
        ptr->next = _new;
    }

    type& operator[](int key) {
        Node<type>* ptr = front;
        for(int i = 0; i < key; i++) {
            ptr = ptr->next;
        }

        return ptr->data;
    }

    ~Linked_list() {
        Node<type>* next = front;
        Node<type>* temp;
        while(next != 0) {
            temp = next->next;
            delete next;
            next = temp;
        }
    }
};

template<typename keytype, typename valtype>
struct Hashmap {
    Dynamic_array<Linked_list<std::pair<keytype, valtype>>>* table = new Dynamic_array<Linked_list<std::pair<keytype, valtype>>>(16);
    int size = 0;

    uint32_t hash_function(keytype key) {
        uint32_t hash = 0;

        uint32_t key_size = sizeof(keytype);
        uint8_t* key_ptr = (uint8_t*)&key;
        for(int i = 0; i < key_size; i++) {
            hash += (uint32_t)key_ptr[i] << ((i % 4) * 8);
        }

        return hash % table->size;
    }

    void insert(keytype key, valtype value) {
        int hash_value = hash_function(key);
        auto& list = (*table)[hash_value];
        Node<std::pair<keytype, valtype>>* ptr = list.front;

        bool key_exists = false;
        
        for(; ptr != 0; ptr = ptr->next) {
            if(ptr->data.first == key) {
                // key exists
                key_exists = true;
                ptr->data.second = value;
                std::cout << "key has been replaced\n";
                break;
            }
        }

        if(!key_exists) {
            list.push_front(std::pair(key, value));
            size++;
        }

        // rehash all values
        if(size >= 0.75 * table->size) {
            Dynamic_array<Linked_list<std::pair<keytype, valtype>>>* old_table = table;
            table = new Dynamic_array<Linked_list<std::pair<keytype, valtype>>>(old_table->size * 2);

            for(int i = 0; i < old_table->size; i++) {
                auto& list = old_table->data[i];
                Node<std::pair<keytype, valtype>>* ptr = list.front;
                while(ptr != 0) {
                    int hash_value = hash_function(ptr->data.first);
                    (*table)[hash_value].push_front(ptr->data);
                    ptr = ptr->next;
                }
            }
            delete old_table;
        }
    }

    void remove(keytype key) {
        int hash_value = hash_function(key);
        auto& list = table[hash_value];
        Node<std::pair<keytype, valtype>>* ptr = list.front;
        Node<std::pair<keytype, valtype>>** prev_ptr = &list.front; 

        bool key_exists = false;
        
        while(true) {
            if(ptr == 0) {
                std::cout << "item not found\n";
                break;
            } else if(ptr->data.first == key) {
                // key exists
                *prev_ptr = ptr->next;
                delete ptr;
                
                std::cout << "item has been removed\n";
                break;
            } else {
                prev_ptr = &ptr->next;
                ptr = ptr->next;
            }
        }
    }

    valtype& operator[](keytype key) {
        int hash_value = hash_function(key);
        auto& list =(*table)[hash_value];
        Node<std::pair<keytype, valtype>>* ptr = list.front;

        bool key_exists = false;
        
        for(; ptr != 0; ptr = ptr->next) {
            if(ptr->data.first == key) {
                // key exists
                key_exists = true;
                return ptr->data.second;
            }
        }

        if(!key_exists) {
            std::cout << "key does not exist\n";
        }
    }

    void print() {
        /*for(int i = 0; i < table->size; i++) {
            Node<std::pair<keytype, valtype>>* next = table[i].front;

            while(next != 0) {
                std::cout << next->data.first << " : " << next->data.second << "   ";
                next = next->next;
            }

            std::cout << std::endl;
        }*/

        for(int i = 0; i < table->size; i++) {
            std::cout << (*table)[i].size << " ";
        }
        std::cout << std::endl;

        /*int min = 0x7fffffff;
        int max = 0;
        for(int i = 0; i < table->size; i++) {
            int size = (*table)[i].size;
            if(size != 0) min = std::min(size, min);
            max = std::max((*table)[i].size, max);
        }
        std::cout << min << " " << max << " " << table->size << std::endl;*/
    }

    bool is_empty() {
        for(int i = 0; i < table->size; i++) {
            if(table[i].size != 0) {
                return false;
            }
        }
        return true;
    }

    ~Hashmap() {
        delete table;
    }
};

int main() {
    Hashmap<int, int> hashmap;
    std::map<int, int> hashmap2;

    if(hashmap.is_empty()) {
        std::cout << "Map is empty\n";
    }

    int s = 0x100;

    time_t start_time = get_time();

    for(int i = 0; i < s; i++) {
        //std::array<short, 2> key = {short(i % 256), short(i / 256)};
        hashmap.insert(i, i);
    }

    time_t end_time = get_time();

    std::cout << double(end_time - start_time) / 1000000000 << " (ave)" << std::endl;

    start_time = get_time();

    for(int i = 0; i < s; i++) {
        //std::array<short, 2> key = {short(i % 256), short(i / 256)};
        int num = hashmap[i];
    }

    end_time = get_time();

    std::cout << double(end_time - start_time) / 1000000000 << " (ave)" << std::endl;

    //

    start_time = get_time();

    for(int i = 0; i < s; i++) {
        //std::array<short, 2> key = {short(i % 256), short(i / 256)};
        hashmap2.emplace(i, i);
    }

    end_time = get_time();

    std::cout << double(end_time - start_time) / 1000000000 << " (std)" << std::endl;

    start_time = get_time();

    for(int i = 0; i < s; i++) {
        //std::array<short, 2> key = {short(i % 256), short(i / 256)};
        int num = hashmap2.at(i);
    }

    end_time = get_time();

    std::cout << double(end_time - start_time) / 1000000000 << " (std)" << std::endl;

    hashmap.print();

    return 0;
}