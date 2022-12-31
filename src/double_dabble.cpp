#include <iostream>
#include <cmath>
#include <array>

int main() {
    unsigned int number;
    std::cin >> number;

    int digits = std::max(ceil(ceil(std::log10(number)) / 2.0), 1.0);

    std::cout << "number: " << number << "\n" << "digits: " << digits << std::endl;

    uint8_t* bits = new uint8_t[digits];

    for(int i = 0; i < digits; i++) {
        bits[i] = 0;
    }

    for(int i = 0; i < 32; i++) {
        uint8_t overflow_number = (number >> 31);
        number <<= 1;

        uint8_t prev_overflow = 0;
        uint8_t overflow;
        for(int j = 0; j < digits; j++) {
            overflow = bits[j] >> 7;
            bits[j] <<= 1;
            bits[j] += prev_overflow;
            prev_overflow = overflow;
        }
        bits[0] += overflow_number; 

        if(i != 31) {
            for(int j = 0; j < digits; j++) {
                if((bits[j] & 15) > 4) {
                    bits[j] += 3;
                }
                if(((bits[j] >> 4) & 15) > 4) {
                    bits[j] += 0x30;
                }
            }
        }
    }

    for(int i = 0; i < digits; i++) {
        std::cout << std::hex << (int)bits[digits - 1 - i];
    }

    delete bits;
}