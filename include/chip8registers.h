#ifndef CHIP8REGISTERS_H
#define CHIP8REGISTERS_H

#include "config.h"
struct chip8_registers
{
    // 16 general purpose 8-bit registers (Vx), where x is a hexadecimal digit (0 through F)
    unsigned char V[CHIP8_TOTAL_DATA_REGISTERS];

    // 16-bit register called I. This register is generally used to store memory addresses, so only the lowest (rightmost) 12 bits are usually used.
    unsigned short I;

    // Two special purpose 8-bit registers, for the delay and sound timers. When these registers are non-zero, they are automatically decremented at a rate of 60Hz.
    unsigned char delay_timer;
    unsigned char sound_timer;

    // The program counter (PC) is 16-bit, and is used to store the currently executing address.
    unsigned short PC;

    // The stack pointer (SP) can be 8-bit, it is used to point to the topmost level of the stack.
    unsigned char SP;
};

#endif