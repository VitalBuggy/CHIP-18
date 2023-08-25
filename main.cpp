#include "core/Chip8.cpp"
#include "platform/Platform.cpp"
#include <iostream>

int main(int argc, char **argv) {
    int vscale = std::stoi(argv[1]);
    int cdelay = std::stoi(argv[2]);
    char const* rom = argv[3];

    Platform p("Chip-8", VIDEO_WIDTH * vscale, VIDEO_HEIGHT * vscale, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 chip;
    chip.LoadROM(rom);

    int videopitch = sizeof(chip.display[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while(!quit) {
        quit = p.process_input(chip.keys);
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cdelay) {
            lastCycleTime = currentTime;
            chip.Cycle();
            p.update(chip.display, videopitch);
        }
    }


    return 0;
}