#ifndef CHIP8MEMORY_H
#define CHIP8MEMORY_H

// Memory Map:
// +---------------+= 0xFFF (4095) End of Chip-8 RAM
// |               |
// |               |
// |               |
// |               |
// |               |
// | 0x200 to 0xFFF|
// |     Chip-8    |
// | Program / Data|
// |     Space     |
// |               |
// |               |
// |               |
// +- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
// |               |
// |               |
// |               |
// +---------------+= 0x200 (512) Start of most Chip-8 programs
// | 0x000 to 0x1FF|
// | Reserved for  |
// |  interpreter  |
// +---------------+= 0x000 (0) Start of Chip-8 RAM

#include "config.h"
struct chip8_memory
{
    unsigned char memory[CHIP8_MEMORY_SIZE];
};

void chip8_memory_set(struct chip8_memory* memory, int index, unsigned char val);
unsigned char chip8_memory_get(struct chip8_memory* memory, int index);
unsigned short chip8_memory_get_short(struct chip8_memory* memory, int index);

#endif