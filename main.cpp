#include "core/Chip8.cpp"
#include <iostream>

int main() {

    Chip8 chip{};
    chip.LoadROM("test.ch8");

    for (int i = 0x00; i < 0xFFF; i++) {
        std::cout << std::hex << static_cast<int>(chip.memory[i]) << ' ';
    }

    return 0;
}