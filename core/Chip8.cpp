#include <cstdint>
#include <fstream>
#include <cstring>
#include <chrono>
#include <random>
#include "FontSet.hpp"

#define START_ADDRESS 0x200
#define FONT_SET_START_ADDRESS 0x50
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32

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

    // tables
    typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

    void T0();
    void T8();
    void TE();
    void TF();
    void OP_NULL();

    void LoadROM(char const *filename);

    void Cycle();

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
    void OP_Annn(); // LD I, addr
    void OP_Bnnn(); // JP V0, addr
    void OP_Cxnn(); // RND Vx, byte
    void OP_Dxyn(); // DRW Vx, Vy, N
    void OP_Ex9E(); // SKP Vx
    void OP_ExA1(); // SKNP Vx
    void OP_Fx07(); // LD Vx, DT
    void OP_Fx0A(); // LD Vx, K
    void OP_Fx15(); // LD DT, Vx
    void OP_Fx18(); // LD ST, Vx
    void OP_Fx1E(); // ADD I, Vx
    void OP_Fx29(); // LD F, Vx
    void OP_Fx33(); // LD B, Vx
    void OP_Fx55(); // LD [I], Vx
    void OP_Fx65(); // LD Vx, [I]

    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

    Chip8();
};

Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    FontSet f;

    table[0x0] = &Chip8::T0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::T8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxnn;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TE;
    table[0xF] = &Chip8::TF;

    for (size_t i = 0; i <= 0xE; i++) {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    for (size_t i = 0; i <= 0x65; i++) {
        tableF[i] = &Chip8::OP_NULL;
    }

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;

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

    if (this->registers[r1] != this->registers[r2])
        this->pc += 2;
}

void Chip8::OP_Annn()
{
    uint16_t addr = this->opcode & 0x0FFFu;
    this->index = addr;
}

void Chip8::OP_Bnnn()
{
    uint16_t addr = this->opcode & 0x0FFFu;
    this->pc = addr + this->registers[0];
}

void Chip8::OP_Cxnn()
{
    uint8_t r1 = (this->opcode & 0x0F00u) >> 8u;
    uint8_t mask = this->opcode & 0x00FFu;

    this->registers[r1] = randByte(randGen) & mask;
}

void Chip8::OP_Dxyn()
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;
    uint8_t height = this->opcode & 0x000Fu;

    uint8_t x = this->registers[Vx];
    uint8_t y = this->registers[Vy];

    this->registers[0xF] = 0;

    for (unsigned int i = 0; i < height; i++)
    {
        uint8_t sprite_byte = this->memory[this->index + i];
        for (unsigned int j = 0; j < 8; j++)
        {
            uint8_t spritePixel = sprite_byte & (0x80u >> j);
            uint32_t *screenPixel = &display[(y + i) * VIDEO_WIDTH + (x + j)];

            // Sprite pixel is on
            if (spritePixel)
            {
                // Screen pixel also on - collision
                if (*screenPixel == 0xFFFFFFFF)
                {
                    this->registers[0xF] = 1;
                }

                // Effectively XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::OP_Ex9E()
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

    uint8_t key = this->registers[Vx];

    if (this->keys[key])
    {
        pc += 2;
    }
}

void Chip8::OP_ExA1()
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

    uint8_t key = this->registers[Vx];

    if (!this->keys[key])
    {
        pc += 2;
    }
}

void Chip8::OP_Fx07()
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

    this->registers[Vx] = this->dtimer;
}

void Chip8::OP_Fx0A()
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

    bool set = false;

    for (int i = 0; i < 16; i++)
    {
        if (this->keys[i])
        {
            this->registers[Vx] = i;
            set = true;
            break;
        }
    }

    if (!set)
        this->pc -= 2;
}

void Chip8::OP_Fx15() // LD DT, Vx
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

    this->dtimer = this->registers[Vx];
}

void Chip8::OP_Fx18() // LD DT, Vx
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

    this->stimer = this->registers[Vx];
}

void Chip8::OP_Fx1E() // LD I, Vx
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

    this->index += this->registers[Vx];
}

void Chip8::OP_Fx29() // LD F, Vx
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
    uint8_t digit = this->registers[Vx];

    this->index = FONT_SET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33() // LD B, Vx
{
    uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
    uint8_t number = this->registers[Vx];

    this->memory[this->index + 2] = number % 10;
    number /= 10;

    this->memory[this->index + 1] = number % 10;
    number /= 10;

    this->memory[this->index] = number % 10;
}

void Chip8::OP_Fx55() // LD [I], Vx
{
    uint8_t Vx = (this->opcode & 0X0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; i++)
    {
        this->memory[this->index + i] = this->registers[i];
    }
}

void Chip8::OP_Fx65() // LD Vx, [I]
{
    uint8_t Vx = (this->opcode & 0X0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; i++)
    {
        this->registers[i] = this->memory[this->index + i];
    }
}

void Chip8::T0() {
    ((*this).*(this->table0[this->opcode & 0x000Fu]))();
}

void Chip8::T8() {
    ((*this).*(this->table8[this->opcode & 0x000Fu]))();
}

void Chip8::TE() {
    ((*this).*(this->tableE[this->opcode & 0x000Fu]))();
}

void Chip8::TF() {
    ((*this).*(this->tableF[this->opcode & 0x000Fu]))();
}

void Chip8::OP_NULL() { }

void Chip8::Cycle()
{
    this->opcode = (this->memory[this->pc] << 8u | this->memory[this->pc + 1]);
    
    this->pc += 2;
    
    // evil pointer hack
    ((*this).*(this->table[(this->opcode & 0xf000u) >> 12u]))();

    if (this->dtimer > 0) --this->dtimer;
    if (this->stimer > 0) --this->stimer;
}