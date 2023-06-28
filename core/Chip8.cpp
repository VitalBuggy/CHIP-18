#include <cstdint>
#include <fstream>
#include <iostream>
#include <cstring>
#include "FontSet.hpp"

#define START_ADDRESS 0x200
#define FONT_SET_START_ADDRESS 0x50

class Chip8
{
public:
    uint8_t registers[16]{};
    uint8_t memory[4096]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t sp{};
    uint8_t dtimer{}; // delay timer
    uint8_t stimer{}; // sound timer
    uint8_t keys[16]{};
    uint32_t display[64 * 32]{};
    uint16_t opcode;

    void LoadROM(char const *filename);

    void OP_00E0(); // CLS
    void OP_00EE(); // RET
    void OP_1nnn(); // JP addr
    void OP_2nnn(); // CALL addr
    void OP_3xkk(); // CALL addr
    Chip8();
};

Chip8::Chip8()
{
    FontSet f;

    for (unsigned int i = 0; i < f.FONTSET_SIZE; i++)
    {
        memory[FONT_SET_START_ADDRESS + i] = f.fontset[i];
    }

    pc = START_ADDRESS;
}

void Chip8::LoadROM(char const *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        std::streampos size = file.tellg();
        char *buffer = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i = 0; i < size; i++)
        {
            this->memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    }
}

void Chip8::OP_00E0()
{
    std::memset(this->display, 0, sizeof(this->display));
}

void Chip8::OP_00EE()
{
    this->sp--;
    this->pc = this->stack[this->sp];
}

void Chip8::OP_1nnn()
{
    uint8_t addr = this->opcode & 0x0FFFu;

    this->pc = addr;
}

void Chip8::OP_2nnn()
{
    uint8_t addr = this->opcode & 0x0FFFu;

    this->stack[this->sp] = this->pc;
    this->sp++;

    this->pc = addr;
}

void Chip8::OP_3xkk()
{
    uint8_t reg = (this->opcode & 0x0F00u) >> 8u;
    uint16_t data = this->opcode & 0x00FF;

    if (this->registers[reg] == data)
    {
        this->pc += 2;
    }
}