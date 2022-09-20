#include "chip8screen.h"
#include <assert.h>
#include <memory.h>

//checks if (x, y) is out of bound
static void chip_screen_check_bounds(int x, int y)
{
    assert(x >= 0 && x < CHIP8_WIDTH && y >= 0 && y < CHIP8_HEIGHT);
}

//clears the screen to blank state (all black)
void chip8_screen_clear(struct chip8_screen* screen)
{
    memset(screen->pixels, 0, sizeof(screen->pixels));
}

//set the pixel at (x, y) to white
void chip8_screen_set(struct chip8_screen* screen, int x, int y)
{
    chip8_screen_check_bounds(x, y);
    screen->pixels[y][x] = true;
}

//checks if the pixel at (x, y) is white
bool chip8_screen_is_set(struct chip8_screen* screen, int x, int y)
{
    chip8_screen_check_bounds(x, y);
    return screen->pixels[y][x];
}

//draws one of the sprites from 0~F to the screen. The top-left corner is at (x, y).
//num = number of rows need to draw in a sprite
bool chip8_screen_draw_sprite(struct chip8_screen* screen, int x, int y, const char* sprite, int num)
{
    // no collision at first
    bool pixel_collision = false;
    for (int sprite_y = 0; sprite_y < num; sprite_y++)
    {
        char c = sprite[sprite_y];
        for (int shift_bits = 0; shift_bits < 8; shift_bits++)
        {
            //check whether the current bit is off
            if ((c & (0b10000000 >> shift_bits)) == 0)
                continue;
            //if the spite is positioned outside the screen partly, wraps around to the opposite side
            int lx = (x + shift_bits) % CHIP8_WIDTH;
            int ly = (y + sprite_y) % CHIP8_HEIGHT;
            if (screen->pixels[ly][lx])
            {
                pixel_collision = true;
            }
            screen->pixels[ly][lx] ^= true;
        }
    }
    return pixel_collision;
}
