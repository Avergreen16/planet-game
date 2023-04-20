#include <thread>
#include <mutex>
#include <iostream>

std::mutex mutex;

void thread_func() {
    while(true) {
        for(int i = 0; i < 24; i++) {
            std::cout << i << " ";
        }
        std::cout << "b\n";
    }
}

int main() {
    std::thread t(
        []() {
            thread_func();
        }
    );

    while(true) {
        for(int i = 0; i < 24; i++) {
            std::cout << i << " ";
        }
        std::cout << "a\n";
    }
}