#include "chip8.h"
#include <memory.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "SDL.h"

const char chip8_default_character_set[] =
{
    0xf0, 0x90, 0x90, 0x90, 0xf0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xf0, 0x10, 0xf0, 0x80, 0xf0, //2
    0xf0, 0x10, 0xf0, 0x10, 0xf0, //3
    0x90, 0x90, 0xf0, 0x10, 0x10, //4
    0xf0, 0x80, 0xf0, 0x10, 0xf0, //5
    0xf0, 0x80, 0xf0, 0x90, 0xf0, //6
    0xf0, 0x10, 0x20, 0x40, 0x40, //7
    0xf0, 0x90, 0xf0, 0x90, 0xf0, //8
    0xf0, 0x90, 0xf0, 0x10, 0xf0, //9
    0xf0, 0x90, 0xf0, 0x90, 0x90, //A
    0xe0, 0x90, 0xe0, 0x90, 0xe0, //B
    0xf0, 0x80, 0x80, 0x80, 0xf0, //C
    0xe0, 0x90, 0x90, 0x90, 0xe0, //D
    0xf0, 0x80, 0xf0, 0x80, 0xf0, //E
    0xf0, 0x80, 0xf0, 0x80, 0x80  //F
};

//copy the sprites to the memory, starting from 0x00
void chip8_init(struct chip8* chip8)
{
    memset(chip8, 0, sizeof(struct chip8));
    memcpy(*chip8->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}

void chip8_load(struct chip8* chip8, const char* buf, size_t size)
{
    //check if the program size is too large
    assert (size + CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
    memcpy (&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
    //set the PC at the start of the program
    chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}

//all execution stops until a key is pressed
static char chip8_wait_for_keypress(struct chip8* chip8)
{
    SDL_Event event;
    while (SDL_WaitEvent(&event))
    {
        if (event.type != SDL_KEYDOWN)
        {
            continue;
        }
        // a key is pressed
        char c = event.key.keysym.sym;
        // maps the key to the chip8 keyboard
        char chip8_key = chip8_keyboard_map(&chip8->keyboard, c);
        if (chip8_key != -1)
        {
            return chip8_key;
        }
    }
    return -1;
}

static void chip8_exec_case_eight(struct chip8* chip8, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000f;
    unsigned char y = (opcode >> 4) & 0x000f;
    unsigned char n = opcode & 0x000f;    
    switch (n)
    {
        // (8xy0) LD Vx, Vy: set Vx = Vy
        case 0x00:
            chip8->registers.V[x] = chip8->registers.V[y];
        break;

        // (8xy1) OR Vx, Vy: set Vx = Vx OR VY
        case 0x01:
            chip8->registers.V[x] |= chip8->registers.V[y];
        break;

        // (8xy2) OR Vx, Vy: set Vx = Vx AND VY
        case 0x02:
            chip8->registers.V[x] &= chip8->registers.V[y];
        break;

        // (8xy3) OR Vx, Vy: set Vx = Vx XOR VY
        case 0x03:
            chip8->registers.V[x] ^= chip8->registers.V[y];
        break;

        // (8xy4) ADD Vx, Vy: set Vx = Vx + VY, VF = carry
        case 0x04:
            unsigned int temp = chip8->registers.V[x] + chip8->registers.V[y];
            chip8->registers.V[0x0f] = 0;
            if (temp > 0xff)
            {
                chip8->registers.V[0x0f] = 1;
            }
            chip8->registers.V[x] = temp % 0xff;
        break;

        // (8xy5) SUB Vx, Vy: set Vx = Vx - Vy, VF = NOT borrow
        case 0x05:
            chip8->registers.V[0x0f] = chip8->registers.V[x] > chip8->registers.V[y];
            chip8->registers.V[x] -= chip8->registers.V[y];
        break;

        // (8xy6) SHR Vx {, Vy}: set Vx = Vx SHR 1
        case 0x06:
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0x01;
            chip8->registers.V[x] /= 2;
        break;

        // (8xy7) SUBN Vx, Vy: set Vx = Vy - Vx, VF = NOT borrow
        case 0x07:
            chip8->registers.V[0x0f] = chip8->registers.V[x] < chip8->registers.V[y];
            chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
        break; 

        // (8xyE) SHL Vx {, Vy}: set Vx = Vx SHL 1
        case 0x0E:
            chip8->registers.V[0x0f] = (chip8->registers.V[x] >> 7) & 0x01;
            chip8->registers.V[x] *= 2;
        break;     
    }
}

static void chip8_exec_case_E(struct chip8* chip8, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000f;
    unsigned char Vx = chip8->registers.V[x];
    switch (opcode & 0x00ff)
    {
        // (Ex9E) SKP Vx: skip next instruction if key with the value of Vx is pressed
        case 0x9E:
            if (chip8_keyboard_is_down(&chip8->keyboard, Vx))
            {
                chip8->registers.PC += 2;
            }
        break;

        // (Exa1) SKNP Vx: skip next instruction if key with the value of Vx is not pressed
        case 0xA1:
            if (!chip8_keyboard_is_down(&chip8->keyboard, Vx))
            {
                chip8->registers.PC += 2;
            }
        break;
    }
}

static void chip8_exec_case_F(struct chip8* chip8, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000f;
    unsigned char Vx = chip8->registers.V[x];
    unsigned short I = chip8->registers.I;
    switch (opcode & 0x00ff)
    {
        // (Fx07) LD Vx, DT: set Vx = delay timer value
        case 0x07:
            chip8->registers.V[x] = chip8->registers.delay_timer;
        break;

        // (Fx0A) LD Vx, K: wait for a key press, store the value of the key in Vx
        case 0x0A:
            char pressed_key = chip8_wait_for_key_press(chip8);
            chip8->registers.V[x] = pressed_key;
        break;

        // (Fx15) LD DT, Vx: set delay timer = Vx
        case 0x15:
            chip8->registers.delay_timer = Vx;
        break;

        // (Fx18) LD ST, Vx: set sound timer = Vx
        case 0x18:
            chip8->registers.sound_timer = Vx;
        break;

        // (Fx1E) ADD I, Vx: set I = I + Vx
        case 0x1E:
            chip8->registers.I += Vx;
        break;

        // (Fx29) LD F, Vx: set I = location of sprite for digit Vx
        case 0x29:
            chip8->registers.I = Vx * CHIP8_DEFAULT_SPRITE_HEIGHT;
        break;

        // (Fx33) LD B, Vx: store BCD representation of Vx in memory locations I, I+1, I+2
        case 0x33:
            unsigned char hundreds = Vx / 100;
            unsigned char tens = (Vx % 100) / 10;
            unsigned char ones = Vx % 10;
            chip8_memory_set(&chip8->memory, I, tens);
            chip8_memory_set(&chip8->memory, I + 1, hundreds);
            chip8_memory_set(&chip8->memory, I + 2, ones);
        break;

        // (Fx55) LD [I], Vx: store registers V0 ~ Vx in memory starting at location I
        case 0x55:
        {
            for (int i = 0; i <= x; i++)
            {
                chip8_memory_set(&chip8->memory, I + i, chip8->registers.V[i]);
            }
        }
        break;

        // (Fx65) LD Vx, [i]: read registers V0  Vx from memory starting at location I
        case 0x65:
        {
            for (int i = 0; i <= x; i++)
            {
                chip8->registers.V[i] = chip8_memory_get(&chip8->memory, I + i);
            }
        }
        break;
    }

}

static void chip8_exec_extended(struct chip8* chip8, unsigned short opcode)
{
    unsigned short nnn = opcode & 0x0fff;
    unsigned char x = (opcode >> 8) & 0x000f;
    unsigned char y = (opcode >> 4) & 0x000f;
    unsigned char n = opcode & 0x000f;    
    unsigned char kk = opcode && 0x00ff;

    unsigned char Vx = chip8->registers.V[x];
    unsigned char Vy = chip8->registers.V[x];

    switch (opcode & 0xf000)
    {
        // (1nnn) JP addr: jump to location nnn
        case 0x1000:
            chip8->registers.PC = nnn;
        break;

        // (2nnn) CALL addr: call subroutine at nnn
        case 0x2000:
            chip8->registers.SP++;
            chip8_stack_push(chip8, chip8->registers.PC);
            chip8->registers.PC = nnn;
        break;

        // (3xkk) SE Vx, byte: skip next instruction if Vx = kk
        case 0x3000:
            if (Vx == kk)
            {
                chip8->registers.PC += 2;
            }
        break;

        // (4xkk) SNE Vx, byte: skip next instruction if Vx != kk
        case 0x4000:
            if (Vx != kk)
            {
                chip8->registers.PC += 2;
            }
        break;

        // (5xkk) SE Vx, Vy: skip next instruction if Vx = Vy
        case 0x5000:
            if (Vx == Vy)
            {
                chip8->registers.PC += 2;
            }
        break;

        // (6xkk) LD Vx, byte: set Vx = kk
        case 0x6000:
            chip8->registers.V[x] = kk;
        break;

        // (7xkk) ADD Vx, byte: set Vx = Vx + kk
        case 0x7000:
            chip8->registers.V[x] += kk;
        break;        

        case 0x8000:
            chip8_exec_case_eight(chip8, opcode);
        break;

        // (9xy0) SNE Vx, Vy: skip next instruction if Vx != Vy
        case 0x9000:
            if (Vx != Vy)
            {
                chip8->registers.PC += 2;
            }            
        break;

        // (Annn) LD I, addr: set I = nnn
        case 0xA000:
            chip8->registers.I = nnn;
        break;

        // (Bnnn) JP V0, addr: jump to locaation nnn + V0
        case 0xB000:
            chip8->registers.PC = nnn + chip8->registers.V[0x00];
        break;

        // (Cxkk) RND Vx, byte: set Vx = random byte AND kk
        case 0xC000:
            srand(clock());
            chip8->registers.V[x] = (rand() % 0xff) & kk;
        break;

        // (Dxyn) DRW Vx, Vy, nibble: display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
        case 0xD000:
            const char* sprite = (const char*) &chip8->memory.memory[chip8->registers.I];
            chip8->registers.V[0x0f] = chip8_screen_draw_sprite(&chip8->screen, Vx, Vy, sprite, n);
        break;

        case 0xE000:
            chip8_exec_case_E(chip8, opcode);
        break;

        case 0xF000:
            chip8_exec_case_F(chip8, opcode);
        break;        
    }
}

void chip8_exec(struct chip8* chip8, unsigned short opcode)
{
    switch(opcode)
    {
        // CLS: clear the display
        case 0x00E0:
            chip8_screen_cleaar(&chip8->screen);
        break;

        // RET: return from subroutine
        case 0x00EE:
            chip8->registers.PC = chip8_stack_pop(chip8);
        break;

        default:
            chip8_exec_extended(chip8, opcode);
    }
}

