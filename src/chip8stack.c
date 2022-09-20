#include "chip8stack.h"
#include "chip8.h"
#include <assert.h>

//checks if the depth of the stack is in bound
static void chip8_stack_in_bounds(struct chip8* chip8)
{
    //SP is unsigned, no need to check >=0
    assert(chip8->registers.SP < CHIP8_TOTAL_STACK_DEPTH);
}

//writes to the memory at current stack pointer. Pointer moves to next depth.
void chip8_stack_push(struct chip8* chip8, unsigned short val)
{
    //checks if the depth of the stack is in bound. Exit if it's not.
    chip8_stack_in_bounds(chip8);
    chip8->stack.stack[chip8->registers.SP] = val;
    chip8->registers.SP++;
}

//pops the data at current stack pointer.
unsigned short chip8_stack_pop(struct chip8* chip8)
{
    chip8->registers.SP--;
    //checks if the depth of the stack is in bound. Exit if it's not.
    chip8_stack_in_bounds(chip8);
    return chip8->stack.stack[chip8->registers.SP];
}
