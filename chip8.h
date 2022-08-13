#ifndef CHIP8_H
#define CHIP8_H
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int uCharArrayDifference(unsigned char *arr1, unsigned char *arr2, int size);

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
    unsigned short drawFlag;

    // timers that count at 60Hz
    unsigned char delay_timer;
    unsigned char sound_timer;

    // stack and stack pointer
    unsigned short stack[16];
    unsigned short sp;

    // keypad state
    unsigned char key[16];
    unsigned char savedKeyState[16];
    unsigned short inputBlockingFlag;
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
    chip8.drawFlag = 0;
    chip8.inputBlockingFlag = 0;

    // clear display
    for (int i = 0; i < 64*32; i++)
    {
        chip8.gfx[i] = 0;
    }
    // clear stack
    for (int i = 0; i < 16; i++)
    {
        chip8.stack[i] = 0x00;
    }
    // clear registers V0-VF
    for (int i = 0; i < 16; i++)
    {
        chip8.V[i] = 0x00;
    }
    // clear memory
    for (int i = 0; i < 4096; i++)
    {
        chip8.memory[i] = 0x00;
    }
    // load fontset
    for (int i = 80; i < (80 + 80); i++)
    {
        chip8.memory[i] = chip8_fontset[i];
    }

    // clear keystates
    for (int i = 0; i < 16; i++)
    {
        chip8.key[i] = 0x00;
    }
    for (int i = 0; i < 16; i++)
    {
        chip8.savedKeyState[i] = 0x00;
    }

    // seed rand()
    srand(time(NULL));

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
        // not enough info in first 4 bits when opcode starts with 0 -- have to go deeper
        case 0x0000:
        {
            switch (chip8->opcode & 0x0FFF)
            {
                case 0x00E0: // 00E0 - clear display
                    for (int i = 0; i < 64*32; i++)
                            chip8->gfx[i] = 0;
                    chip8->pc += 2;
                    break;
                case 0x00EE: // 00EE - return from subroutine
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp] + 2;
                    break;
                default: // 0NNN - call machine code routine
                    // todo
                    printf("Unimplemented 0NNN\n");
            }
            break;
        }
        case 0x1000: // 1NNN - jumps to address NNN
        {
            chip8->pc = chip8->opcode & 0x0FFF;
            break;
        }
        case 0x2000: // 2NNN calls subroutine at NNN
        {
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = chip8->opcode & 0x0FFF;
            break;
        }
        case 0x3000: // 3XNN skip next instruction if VX == NN
            if (chip8->V[chip8->opcode & 0x0F00 >> 8] == (chip8->opcode & 0x0FF))
                chip8->pc += 4;
            else
                chip8->pc += 2;
            break;
        case 0x4000: // 4XNN skip next instruction if VX != NN
            if (chip8->V[(chip8->opcode & 0x0F00) >> 8] != (chip8->opcode & 0x0FF))
                chip8->pc += 4;
            else
                chip8->pc += 2;
            break;
        case 0x5000: // 5XYN skip next instruction if VX == VY
            if (chip8->V[(chip8->opcode & 0x0F00) >> 8] == chip8->V[(chip8->opcode & 0x00F0) >> 4])
                chip8->pc += 4;
            else
                chip8->pc += 2;
            break;
        case 0x6000: // 6XNN Sets VX to NN
            chip8->V[(chip8->opcode & 0x0F00) >> 8] = (chip8->opcode & 0x00FF);
            chip8->pc += 2;
            break;
        case 0x7000: // 7XNN VX += NN (carry flag not set)
            chip8->V[(chip8->opcode & 0x0F00) >> 8] += (chip8->opcode & 0x00FF);
            chip8->pc += 2;
            break;
        // not enough info in first 4 bits when opcode starts with 8 -- have to go deeper
        case 0x8000:
        {
            switch (chip8->opcode & 0x000F)
            {
                case 0x0000: // 8XY0 VX = VY
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    chip8->pc += 2;
                    break;
                case 0x0001: // 8XY1 VX |= VY
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] |= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    chip8->pc += 2;
                    break;
                case 0x0002: // 8XY2 VX &= VY
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] &= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    chip8->pc += 2;
                    break;
                case 0x0003: // 8XY3 VX ^= VY
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] ^= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    chip8->pc += 2;
                    break;
                case 0x0004: // 8XY4 Adds VX to VY, VF set to 1 when there is a carry, 0 when not
                    if (chip8->V[(chip8->opcode & 0x00F0) >> 4] > (0xFF - chip8->V[(chip8->opcode & 0x0F00) >> 8]))
                        chip8->V[0xF] = 1;
                    else
                        chip8->V[0xF] = 0;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] += chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    chip8->pc += 2;
                    break;
                case 0x0005: // 8XY5 Subtracts VY from VX, VF is set to 0 when there is a borrow, 1 when not
                    if (chip8->V[(chip8->opcode & 0x00F0) >> 4] > chip8->V[(chip8->opcode & 0x0F00) >> 8])
                        chip8->V[0xF] = 0;
                    else
                        chip8->V[0xF] = 1;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] -= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    chip8->pc += 2;
                    break;
                case 0x006: // 8XY6 Stores least significant bit of VX in VF, then shifts VX right by one.
                    chip8->V[0xF] = (chip8->V[(chip8->opcode & 0x0F00) >> 8] & -chip8->V[(chip8->opcode & 0x0F00) >> 8]);
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] >>= 1;
                    chip8->pc += 2;
                    break;
                case 0x007: // 8XY7 Sets VX to VY - VX. VF is set to 0 when borrow, 1 when not
                    if (chip8->V[(chip8->opcode & 0x00F0) >> 4] < chip8->V[(chip8->opcode & 0x0F00) >> 8])
                        chip8->V[0xF] = 0;
                    else
                        chip8->V[0xF] = 1;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->V[(chip8->opcode & 0x00F0) >> 4] - chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    chip8->pc += 2;
                    break;
                case 0x00E: // 8XYE Stores least significant bit of VX in VF, then shifts VX left by one.
                    chip8->V[0xF] = (chip8->V[(chip8->opcode & 0x0F00) >> 8] << (chip8->V[(chip8->opcode & 0x0F00) >> 8] - 1));
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] <<= 1;
                    chip8->pc += 2;
                    break;
                default:
                    printf("Unknown opcode: [0x8000]: 0x%X\n", chip8->opcode);
            }
            break;
        }
        case 0x9000: // 9XY0 Skips next instruction if VX != VY
        {
            if (chip8->V[(chip8->opcode & 0x0F00) >> 8] != chip8->V[(chip8->opcode & 0x00F0) >> 4])
                chip8->pc += 4;
            else
                chip8->pc += 2;
        }
        case 0xA000: // ANNN sets I to NNN
        {
            chip8->I = chip8->opcode & 0x0FFF; // gets remaining opcode info for assignment
            chip8->pc += 2;
            break;
        }
        case 0xB000: // BNNN Jumps to NNN + V0
        {
            chip8->pc = chip8->V[0] + (chip8->opcode & 0x0FFF);
        }
        case 0xC000: // CXNN Sets VX to NN & rand[0-255]
        {

            chip8->V[(chip8->opcode & 0x0F00) >> 8] = (chip8->opcode & 0x00FF) & (rand() % 255);
            chip8->pc += 2;
        }
        case 0xD000: // DXYN draws sprite at coords (VX, VY) with width of 8 pixels, height of N pixels,
        // loading sprite from memory location I
        {
            unsigned short x = chip8->V[(chip8->opcode & 0x0F00) >> 8];
            unsigned short y = chip8->V[(chip8->opcode & 0x00F0) >> 4];
            unsigned short height = chip8->opcode & 0x000F;
            unsigned short pixel;

            // register will hold collision flag
            chip8->V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = chip8->memory[chip8->I + yline];
                for (int xline = 0; xline < 8; xline++)
                {
                    if ((pixel & (0x80 >> xline)) != 0)
                    {
                        // check if current display pixel is already set - flag collision
                        if (chip8->gfx[(x + xline + ((y + yline) * 64))] == 1)
                            chip8->V[0xF] = 1;
                        // flip pixel
                        chip8->gfx[(x + xline + ((y + yline) * 64))] ^= 1;
                    }
                }
            }

            chip8->drawFlag = 1;
            chip8->pc += 2;

            break;
        }

        case 0xE000:
        {
            switch (chip8->opcode & 0x00FF)
            {
                case 0x009E: // EX9E skips next instruction if key stored in VX is pressed
                    if (chip8->key[chip8->V[(chip8->opcode & 0x0F00) >> 8]] != 0)
                        chip8->pc += 4;
                    else
                        chip8->pc += 2;
                    break;
                case 0x00A1: // EXA1 skips next instruction if key stored in VX is not pressed
                    if (chip8->key[chip8->V[(chip8->opcode & 0x0F00) >> 8]] == 0)
                        chip8->pc += 4;
                    else
                        chip8->pc += 2;
                    break;
            }
            break;
        }
        case 0xF000:
        {
            switch (chip8->opcode & 0x00FF)
            {
                case 0x0007: // FX07 Set VX to the value of the delay timer
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->delay_timer;
                    chip8->pc += 2;
                    break;
                case 0x000A: // FX0A A key press is awaited (blocking) and then stored in VX
                    if (chip8->inputBlockingFlag == 0)
                    {
                        chip8->inputBlockingFlag = 1;
                        memcpy(chip8->savedKeyState, chip8->key, sizeof(chip8->savedKeyState));
                    }
                    else
                    {
                        int differenceIndex = uCharArrayDifference(chip8->savedKeyState, chip8->key, 16);
                        if (differenceIndex != -1)
                        {
                            chip8->V[(chip8->opcode & 0x0F00) >> 8] = differenceIndex;
                            chip8->inputBlockingFlag = 0;
                            chip8->pc += 2; //(won't increment pc until key press obtained)
                        }
                    }
                    break;
                case 0x0015: // FX15 Set delay timer to VX
                    chip8->delay_timer = chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    chip8->pc += 2;
                    break;
                case 0x0018: // FX18 Set sound timer to VX
                    chip8->sound_timer = chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    chip8->pc += 2;
                    break;
                case 0x001E: // FX1E Add VX to I
                    chip8->I += chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    chip8->pc += 2;
                    break;
                case 0x0029: // FX29 Set I to the location of the sprite for the character VX.
                    chip8->I = chip8->V[(chip8->opcode & 0x0F00) >> 8] * 5;
                    chip8->pc += 2;
                    break;
                case 0x0033: // FX33 Store binary-coded representation of VX, with most significant of digits in I, middle I+1 and last I+2
                    chip8->memory[chip8->I] = (int)chip8->V[(chip8->opcode & 0x0F00) >> 8] % 10;
                    (chip8->memory[chip8->I + 1] = (int)chip8->V[(chip8->opcode & 0x0F00) >> 8] / 10) % 10;
                    (chip8->memory[chip8->I + 2] = (int)chip8->V[(chip8->opcode & 0x0F00) >> 8] / 100) % 10;
                    chip8->pc += 2;
                    break;
                case 0x0055: // FX55 Stores V0 to VX (inclusive) in memory starting at address I in increments of 1. However I does not move.
                    for (int i = 0; i < ((chip8->opcode & 0x0F00) >> 8); i++)
                    {
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    }
                    chip8->pc += 2;
                    break;
                case 0x0065: // FX65 Fills V0 to VX (inclusive) from memory starting at address I in increments of 1. However I does not move.
                    for (int i = 0; i < ((chip8->opcode & 0x0F00) >> 8); i++)
                    {
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    }
                    chip8->pc += 2;
                    break;
            }
            break;
        }
        default:
        {
            printf("Unknown opcode: [0x0000]: 0x%X\n", chip8->opcode);
        }
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

// returns -1 if no difference, otherwise returns index
int uCharArrayDifference(unsigned char *arr1, unsigned char *arr2, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr1[i] != arr2[i])
            return i;
    }
    
    return -1;
}


#endif