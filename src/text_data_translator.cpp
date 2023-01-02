#include <fstream>
#include <vector>

int main() {
    std::ifstream input;
    std::ofstream output;

    input.open("res\\text_data.txt");
    output.open("res\\text_data.bin", std::ios::out | std::ios::trunc | std::ios::binary);

    std::vector<uint8_t> bytes;

    int counter = 0;
    int pos = 0;
    bytes.push_back(9);
    bytes.push_back(10);
    bytes.push_back(1);
    bytes.push_back(0);

    bytes.push_back(0x20);
    bytes.push_back(5);

    int loc = bytes.size();

    while(input.peek() != EOF) {
        int number;
        input >> number;

        bytes.push_back(counter + 0x21 + 1 * (counter + 0x21 >= 0x7F));
        bytes.push_back(number + 1);
        bytes.push_back(pos & 255);
        bytes.push_back(pos >> 8);
        bytes.push_back(number);

        pos += number + 1;

        counter++;
    }

    bytes.insert(bytes.begin() + loc, {uint8_t(counter & 255), uint8_t(counter >> 8)});

    output.write((const char*)bytes.data(), bytes.size());

    input.close();
    output.close();
}