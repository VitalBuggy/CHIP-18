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
    void OP_3xkk(); // SE Vx, byte
    void OP_4xkk(); // SNE Vx, byte
    void OP_5xy0(); // SE Vx, Vy
    void OP_6xkk(); // LD Vx, byte
    void OP_7xkk(); // ADD Vx, byte
    void OP_8xy0(); // LD Vx, Vy
    void OP_8xy1(); // OR Vx, Vy
    void OP_8xy2(); // AND Vx, Vy
    void OP_8xy3(); // XOR Vx, Vy
    void OP_8xy4(); // ADD Vx, Vy
    void OP_8xy5(); // SUB Vx  {, Vy}
    void OP_8xy6(); // SHR Vx, Vy
    void OP_8xy7(); // SUBN Vx, Vy
    void OP_8xyE(); // SHL Vx {, Vy}
    void OP_9xy0(); // SNE Vx, Vy
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
    uint16_t addr = this->opcode & 0x0FFFu;

    this->pc = addr;
}

void Chip8::OP_2nnn()
{
    uint16_t addr = this->opcode & 0x0FFFu;

    this->stack[this->sp] = this->pc;
    this->sp++;

    this->pc = addr;
}

void Chip8::OP_3xkk()
{
    uint8_t reg = (this->opcode & 0x0F00u) >> 8u;
    uint8_t data = this->opcode & 0x00FF;

    if (this->registers[reg] == data)
    {
        this->pc += 2;
    }
}

void Chip8::OP_4xkk()
{
    uint8_t reg = (this->opcode & 0x0F00u) >> 8u;
    uint8_t data = this->opcode & 0x00FF;

    if (this->registers[reg] != data)
    {
        this->pc += 2;
    }
}

void Chip8::OP_5xy0()
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    if (this->registers[r1] == this->registers[r2])
    {
        this->pc += 2;
    }
}

void Chip8::OP_6xkk()
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t data = this->opcode & 0x00FFu;

    this->registers[r1] = data;
}

void Chip8::OP_7xkk()
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t data = this->opcode & 0x00FFu;

    this->registers[r1] += data;
}

void Chip8::OP_8xy0()
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    this->registers[r1] = this->registers[r2];
}

void Chip8::OP_8xy1()
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    this->registers[r1] |= this->registers[r2];
}

void Chip8::OP_8xy2() // AND Vx, Vy
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    this->registers[r1] &= this->registers[r2];
}

void Chip8::OP_8xy3() // XOR Vx, Vy
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    this->registers[r1] ^= this->registers[r2];
}

void Chip8::OP_8xy4() // ADD Vx, Vy
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    uint16_t sum = this->registers[r1] + this->registers[r2];

    this->registers[0xF] = (sum > 255U ? 1 : 0);

    this->registers[r1] = sum & 0xFFu;
}

void Chip8::OP_8xy5() // SUB Vx  {, Vy}
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    this->registers[0xF] = (this->registers[r1] > this->registers[r2] ? 1 : 0);

    this->registers[r1] -= this->registers[r2];
}

void Chip8::OP_8xy6() // SHR Vx, Vy
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    // Set value of VF to the least significant bit of Vy before bit shift and movement to Vx
    this->registers[0xF] = this->registers[r2] & 0x1u;

    // Store the value of Vy shifted to the right by 1 in Vx
    this->registers[r1] = this->registers[r2] >> 1;
}

void Chip8::OP_8xy7() // SUBN Vx, Vy
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u; // Vx
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u; // Vy

    this->registers[0xF] = (this->registers[r1] > this->registers[r2] ? 0 : 1);

    this->registers[r1] = this->registers[r2] - this->registers[r1];
}

void Chip8::OP_8xyE() // SHL Vx {, Vy}
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    // Set value of VF to the most significant bit of Vy before bit shift and movement to Vx
    this->registers[0xF] = (this->registers[r2] & 0x80u) >> 7u;

    // Store the value of Vy shifted to the right by 1 in Vx
    this->registers[r1] = this->registers[r2] << 1;
}

void Chip8::OP_9xy0() // SNE Vx, Vy
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t r2 = (this->opcode & 0x00F0u) >> 4u;

    if (this->registers[r1] != this->registers[r2]) this->pc += 2;
}

