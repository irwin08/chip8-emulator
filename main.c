// Based on excellent article/tutorial here:
//  https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

#include <stdio.h>
#include <SDL2/SDL.h>
#include "chip8.h"

int main()
{

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initiliaze SDL!\n");
        return -1;
    }

    SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer);
    
    Chip8 chip8 = initialize();
    //loadGame(&chip8, "");

    for (;;)
    {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

        SDL_RenderDrawLine(renderer, 5, 5, 35, 35);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}