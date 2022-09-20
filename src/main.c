#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include "SDL.h"
#include "chip8.h"

// a virtual keyboard correspondence
const char keyboard_map[CHIP8_TOTAL_KEYS] = 
{
    '0', '1', '2', '3', 
    '4', '5', '6', '7', 
    '8', '9', 'a', 'b', 
    'c', 'd', 'e', 'f'
};

int main(int argc, char** argv)
{
/* ----------------------------
    BEGIN OF INITIALIZATION
-----------------------------*/

    //check if argc is correct
    if (argc < 2)
    {
        printf("You must provide a file to load\n");
        return -1;
    }
    const char* filename = argv[1];
    printf("The filename to load is: %s\n", filename);

    //load the file into memory
    FILE* f = fopen(filename, "r");
    if (!f)
    {
        printf("Failed to open the file\n");
        return -1;
    }
    //count the size of the file, then return to the beginning
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char buf[size];
    //read the entire file as one chunk
    int res = fread(buf, size, 1, f);
    if (res != 1)
    {
        printf("Failed to read from file\n");
        return -1;
    }
/* ----------------------------
    END OF INITIALIZATION
-----------------------------*/
    struct chip8 chip8;
    chip8_init(&chip8);
    chip8_load(&chip8, buf, size);
    chip8_keyboard_set_map(&chip8.keyboard, &keyboard);

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window=SDL_CreateWindow(
        EMULATOR_WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        CHIP8_WIDTH * CHIP8_WINDOW_MULTIPLIER,
        CHIP8_HEIGHT * CHIP8_WINDOW_MULTIPLIER,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);
    while (1)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    goto out;
                break;

                case SDL_KEYDOWN: 
                {
                    char key = event.key.leysym.sym;
                    char vkey = chip8_keyboard_map(&chip8.keyboard, key);
                    if (vkey != -1)
                    {
                        chip8_keyboard_down(&chip8.keyboard, vkey);
                    }
                }
                break;

                case SDL_KEYUP: 
                {
                    char key = event.key.leysym.sym;
                    char vkey = chip8_keyboard_map(&chip8.keyboard, key);
                    if (vkey != -1)
                    {
                        chip8_keyboard_up(&chip8.keyboard, vkey);
                    }
                }
                break;
            }
        }

        //set background to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        //clear the screen
        SDL_RenderClear(renderer);
        //set draw color to white
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

        //draw color to the pixel if the pixel is on
        for (int x = 0; x <= CHIP8_WIDTH; x++)
        {
            for (int y = 0; y <= CHIP8_HEIGHT; y++)
            {
                if (chip8_screen_is_set(&chip8.screen, x, y))
                {
                    //draw a 10*10 square (multiply the pixel by a multiplier)
                    SDL_Rect r;
                    r.x = x * CHIP8_WINDOW_MULTIPLIER;
                    r.y = y * CHIP8_WINDOW_MULTIPLIER;
                    r.w = CHIP8_WINDOW_MULTIPLIER;
                    r.h = CHIP8_WINDOW_MULTIPLIER;
                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }
        SDL_RenderPresent(renderer);

        //if the delay timer > 0, sleep by 100ms
        if (chip8.registers.delay_timer > 0)
        {
            Sleep(100);
            chp8.registers.delay_timer -= 1;
        }

        //if the sound timer > 0, sleep by 100ms
        if (chip8.registers.sound_timer > 0)
        {
            Beep(25000, 100);
            chp8.registers.sound_timer -= 1;
        }

        //get the opcode and execute
        unsigned short opcode = chip8_memory_get_short(&chip8.memory, chip8.registers.PC);
        chip8.registers.PC += 2; //add 2 bytes every time
        chip8_exec(&chip8, opcode);
    }

out:
    SDL_DestroyWindow(window);
    return 0;
}
