#include "chip8keyboard.h"
#include <assert.h>

//checks if the key is in bound 0~F
static void chip8_keyboard_in_bounds(int key)
{
    assert(key >= 0 && key < CHIP8_TOTAL_KEYS);
}

//sets the virtual keyboard to physical keyboard
void chip8_keyboard_set_map(struct chip8_keyboard* keyboard, const char* map)
{
    keyboard->keyboard_map = map;
}

//matches the pressed physical key with the virtual key
//if no matches, returns -1
int chip8_keyboard_map(struct chip8_keyboard* keyboard, char key)
{
    for (int i = 0; i < CHIP8_TOTAL_KEYS; i++){
        if(keyboard->keyboard_map[i] == key){
            return i;
        }
    }
    return -1;
}

//sets the input key to down/pressed
void chip8_keyboard_down(struct chip8_keyboard* keyboard, int key)
{
    keyboard->keyboard[key] = true;
}

//sets the input key to up/unpressed
void chip8_keyboard_up(struct chip8_keyboard* keyboard, int key)
{
    keyboard->keyboard[key] = false;
}

//returns whether the key is pressed
bool chip8_keyboard_is_down(struct chip8_keyboard* keyboard, int key)
{
    return keyboard->keyboard[key];
}