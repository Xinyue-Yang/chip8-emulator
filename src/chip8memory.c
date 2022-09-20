#include "chip8memory.h"
#include <assert.h>

//checks if the memory is in bounds 0~total memory size
static void chip8_is_memory_in_bounds(int index)
{
    assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}

//given an index, sets the corresponding memory to input value
void chip8_memory_set(struct chip8_memory* memory, int index, unsigned char val)
{
    //in-bound checks
    chip8_is_memory_in_bounds(index);
    memory->memory[index] = val;
}

//given an index, returns the corresponding memory
unsigned char chip8_memory_get(struct chip8_memory* memory, int index)
{
    //in-bound checks
    chip8_is_memory_in_bounds(index);
    return memory->memory[index];
}

//given an index, returns the combination of the the corresponding memory and the memory next to it
unsigned short chip8_memory_get_short(struct chip8_memory* memory, int index)
{
    unsigned char byte1 = chip8_memory_get(memory, index);
    unsigned char byte2 = chip8_memory_get(memory, index + 1);
    return byte1 << 8 | byte2;
}