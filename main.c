// Based on excellent article/tutorial here:
//  https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

#include <stdio.h>
#include "chip8.h"

int main()
{

    Chip8 chip8 = initialize();
    loadGame(&chip8);

    for (;;)
    {
        
    }

    return 0;
}