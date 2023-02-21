#include <iostream>
#include <functional>

struct example;

void times2(example* self);

void squared(example* self);

struct example {
    int value;
    std::function<void(example*)> func;
};

void times2(example* self) {
    std::cout << self->value * 2 << std::endl;
}

void squared(example* self) {
    std::cout << self->value * self->value << std::endl;
}

int main() {
    example A{5, times2};
    example B{16, squared};

    A.func();
    b.func();
}