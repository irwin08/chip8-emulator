// Based on excellent article/tutorial here:
//  https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

#include <stdio.h>
#include <SDL2/SDL.h>
#include "chip8.h"

int main()
{

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Event e;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initiliaze SDL!\n");
        return -1;
    }

    int scale = 10;

    SDL_CreateWindowAndRenderer(64*scale, 32*scale, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, scale, scale);
    
    Chip8 chip8 = initialize();
    //loadGame(&chip8, "");

    int quit = 0;

    while (!quit)
    {
        // user input

        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_UP:
                        break;
                    default:
                        break;
                }
            }
        }

        // cpu stuff

        // drawing

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

        SDL_RenderDrawPoint(renderer, 5, 5);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}