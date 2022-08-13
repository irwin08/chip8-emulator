// Based on excellent article/tutorial here:
//  https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

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

    int frame = 0;
    time_t frame_start = 0;

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
                    case SDLK_1:
                        chip8.key[0] = 1;
                        break;
                    case SDLK_2:
                        chip8.key[1] = 1;
                        break;
                    case SDLK_3:
                        chip8.key[2] = 1;
                        break;
                    case SDLK_4:
                        chip8.key[3] = 1;
                        break;
                    case SDLK_q:
                        chip8.key[4] = 1;
                        break;
                    case SDLK_w:
                        chip8.key[5] = 1;
                        break;
                    case SDLK_e:
                        chip8.key[6] = 1;
                        break;
                    case SDLK_r:
                        chip8.key[7] = 1;
                        break;
                    case SDLK_a:
                        chip8.key[8] = 1;
                        break;
                    case SDLK_s:
                        chip8.key[9] = 1;
                    case SDLK_d:
                        chip8.key[10] = 1;
                    case SDLK_f:
                        chip8.key[11] = 1;
                        break;
                    case SDLK_z:
                        chip8.key[12] = 1;
                        break;
                    case SDLK_x:
                        chip8.key[13] = 1;
                        break;
                    case SDLK_c:
                        chip8.key[14] = 1;
                        break;
                    case SDLK_v:
                        chip8.key[15] = 1;
                        break;
                    default:
                        break;
                }
            }
            else if (e.type == SDL_KEYUP)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_1:
                        chip8.key[0] = 0;
                        break;
                    case SDLK_2:
                        chip8.key[1] = 0;
                        break;
                    case SDLK_3:
                        chip8.key[2] = 0;
                        break;
                    case SDLK_4:
                        chip8.key[3] = 0;
                        break;
                    case SDLK_q:
                        chip8.key[4] = 0;
                        break;
                    case SDLK_w:
                        chip8.key[5] = 0;
                        break;
                    case SDLK_e:
                        chip8.key[6] = 0;
                        break;
                    case SDLK_r:
                        chip8.key[7] = 0;
                        break;
                    case SDLK_a:
                        chip8.key[8] = 0;
                        break;
                    case SDLK_s:
                        chip8.key[9] = 0;
                    case SDLK_d:
                        chip8.key[10] = 0;
                    case SDLK_f:
                        chip8.key[11] = 0;
                        break;
                    case SDLK_z:
                        chip8.key[12] = 0;
                        break;
                    case SDLK_x:
                        chip8.key[13] = 0;
                        break;
                    case SDLK_c:
                        chip8.key[14] = 0;
                        break;
                    case SDLK_v:
                        chip8.key[15] = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        // cpu stuff

        emulateCycle(&chip8);

        frame++;
        // naive 60Hz limit - should try to even out delay in future
        if (frame >= 60)
        {
            int pollingDelay = difftime(time(NULL), frame_start) * 1000; // in milliseconds

            #ifdef _WIN32
            Sleep(pollingDelay);
            #else
            usleep(pollingDelay*1000);  // multiply by 1000 to get microseconds
            #endif

            frame = 0;
            frame_start = time(NULL);
        }

        // drawing

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

        for (int i = 0; i < 32; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                if (chip8.gfx[(i*32) + j] == 1)
                {
                    SDL_RenderDrawPoint(renderer, i, j);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}