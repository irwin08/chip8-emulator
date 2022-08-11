#ifndef CHIP8_H
#define CHIP8_H
#include <stdio.h>

typedef struct Chip8 {
    unsigned short opcode;
    unsigned char memory[4096];
    // registers V0 - VE -- VF reserved for special operations
    unsigned char V[16];
    // index register
    unsigned short I;
    // program counter
    unsigned short pc;

    /* MEMORY MAP
        0x000 - 0x1FF - Chip8 interpreter
        0x050 - 0x0A0 - For 4x5 pixel font set
        0x200 - 0xFFF - Program ROM and work RAM
    */

    // graphics pixels
    unsigned char gfx[64 * 32];

    // timers that count at 60Hz
    unsigned char delay_timer;
    unsigned char sound_timer;

    // stack and stack pointer
    unsigned short stack[16];
    unsigned short sp;

    // keypad state
    unsigned char key[16];
} Chip8;

unsigned char chip8_fontset[80] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8 initialize()
{
    Chip8 chip8;

    chip8.pc = 0x200; // where progam starts as per memory map above
    chip8.opcode = 0;
    chip8.I = 0;
    chip8.sp = 0;

    // clear display
    // clear stack
    // clear registers V0-VF
    // clear memory

    // load fontset
    for (int i = 0; i < 80; i++)
    {
        chip8.memory[i] = chip8_fontset[i];
    }

    return chip8;
}

void loadGame(Chip8 *chip8, char *filePath)
{
    FILE *file = fopen(filePath, "rb");

    // should cut this down
    const size_t bufferSize = 3584;
    unsigned char buffer[bufferSize];

    fread(buffer, sizeof(char), bufferSize, file);

    for (int i = 0; i < bufferSize; i++)
    {
        chip8->memory[i + 512] = buffer[i];
    }
}

void emulateCycle(Chip8 *chip8)
{
    // fetch opcode (grab first 2 bytes of memory and merge)
    chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    // decode opcode (0xF000 AND gets first four bits of two-byte opcode)
    switch (chip8->opcode & 0xF000)
    {
        case 0xA000: // ANNN sets I to NNN
            chip8->I = chip8->opcode & 0x0FFF; // gets remaining opcode info for assignment
            chip8->pc += 2;
            break;

        case 0x2000: // 2NNN calls subroutine at NNN
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = chip8->opcode & 0x0FFF;
            break;

        // not enough info in first 4 bits when opcode starts with 0 -- have to go deeper
        case 0x0000:
            switch (chip8->opcode & 0x000F)
            {
                case 0x0000:
                    break;
                case 0x000E:
                    break;
                default:
                    printf("Unknown opcode: [0x0000]: 0x%X\n", chip8->opcode);
            }
            break;

        // not enough info in first 4 bits when opcode starts with 8 -- have to go deeper
        case 0x8000:
            switch (chip8->opcode & 0x000F)
            {
                case 0x0001:
                    break;
                case 0x0002:
                    break;
                case 0x0003:
                    break;
                case 0x0004: // 8XY4 Adds VX to VY, VF set to 1 when there is a carry, 0 when not
                    if (chip8->V[(chip8->opcode & 0x00F0) >> 4] > (0xFF - chip8->V[(chip8->opcode & 0x0F00) >> 8]))
                        chip8->V[0xF] = 1;
                    else
                        chip8->V[0xF] = 0;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] += chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    chip8->pc += 2;
                    break;
                default:
                    printf("Unknown opcode: [0x8000]: 0x%X\n", chip8->opcode);
            }
            break;
        case 0xF000:
            switch (chip8->opcode & 0x0033)
            {
                case 0x0033: // Store binary-coded representation of VX, with most significant of digits in I, middle I+1 and last I+2
                    chip8->memory[chip8->I] = (int)chip8->V[(chip8->opcode & 0x0F00) >> 8] % 10;
                    (chip8->memory[chip8->I + 1] = (int)chip8->V[(chip8->opcode & 0x0F00) >> 8] / 10) % 10;
                    (chip8->memory[chip8->I + 2] = (int)chip8->V[(chip8->opcode & 0x0F00) >> 8] / 100) % 10;
                    break;
            }
            break;

        default:
            printf("Unknown opcode: [0x0000]: 0x%X\n", chip8->opcode);
        
    }

    // update timers
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;
    if (chip8->sound_timer > 0)
    {
        if (chip8->sound_timer == 1)
        {
            printf("BEEP\n");
        }
        chip8->sound_timer--;
    }
}

void setKeys(Chip8 *chip8)
{

}




#endif