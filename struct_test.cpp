#include <cstring>
#include <array>

template<typename type>
struct Vector {
    type* data = 0;
    size_t size = 0;
    size_t capacity = 0;

    Vector(int _size = 2) {
        size = _size;
        capacity = _size;

        data = new type[_size];
    }

    template<size_t _size>
    Vector& operator=(std::array<type, _size> arr) {
        size = _size;
        capacity = size * 1.5;
        
        if(data != 0) delete[] data;
        data = new type[capacity];
        memcpy(data, arr, size * sizeof(type));
        return *this;
    }

    type& operator[](int index) {
        return data[index];
    }

    void resize(int _size) {
        capacity = _size;
        size = min(size, _size);

        type* temp = new type[_size];
        memcpy(temp, data, size * sizeof(type));
        delete[] data;
        data = temp;
    }

    void push_back(type input) {
        if(size >= capacity) {
            capacity *= 1.5;

            type* temp = new type[capacity];
            memcpy(temp, data, size);
            delete[] data;
            data = temp;
        }

        data[size] = input;
        size++;
    }

    ~Vector() {
        delete[] data;
    }
};

int main() {
    Vector<char*> vector;
    vector.push_back("Hello world!");
    vector.push_back("Avergreen :)");
    vector.push_back("welcome");
}