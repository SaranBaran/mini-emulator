#include <iostream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MEMORY_SIZE 4096
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

class Chip8 {
public:
    uint8_t  memory[MEMORY_SIZE];
    uint8_t  V[16];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[16];
    uint8_t  sp;
    bool display[DISPLAY_WIDTH * DISPLAY_HEIGHT];

    Chip8() {
        pc = 0x200;
        I = 0;
        sp = 0;
        memset(memory, 0, MEMORY_SIZE);
        memset(V, 0, 16);
        memset(display, false, sizeof(display));
    }

    // --- The Graphical Interface: Translating the array to the terminal ---
    void draw() {
        // This escape code clears the terminal screen and moves cursor to top
        printf("\033[H\033[J"); 
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x++) {
                // Draw a solid block for true, spaces for false
                printf(display[y * DISPLAY_WIDTH + x] ? "##" : "  ");
            }
            printf("\n");
        }
    }

    void cycle() {
        uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
        decode(opcode);
    }

    void decode(uint16_t opcode) {
        uint8_t first_nibble = (opcode & 0xF000) >> 12;

        switch(first_nibble) {
            case 0x1: // 1NNN: Jump
                pc = (opcode & 0x0FFF);
                break;
            
            case 0x6: // 6XNN: Set VX to NN
                V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
                pc += 2;
                break;

            case 0xA: // ANNN: Set Index Register I to NNN
                I = (opcode & 0x0FFF);
                pc += 2;
                break;

            case 0xD: { // DXYN: The Draw Instruction
                uint8_t x = V[(opcode & 0x0F00) >> 8] % DISPLAY_WIDTH;
                uint8_t y = V[(opcode & 0x00F0) >> 4] % DISPLAY_HEIGHT;
                uint8_t height = opcode & 0x000F;
                V[0xF] = 0; // Collision flag

                for (int row = 0; row < height; row++) {
                    uint8_t sprite_byte = memory[I + row];
                    for (int col = 0; col < 8; col++) {
                        if ((sprite_byte & (0x80 >> col)) != 0) {
                            int idx = (x + col) + ((y + row) * DISPLAY_WIDTH);
                            if (idx < DISPLAY_WIDTH * DISPLAY_HEIGHT) {
                                if (display[idx]) V[0xF] = 1; 
                                display[idx] ^= true; // XOR!
                            }
                        }
                    }
                }
                pc += 2;
                break;
            }

            default:
                pc += 2; 
                break;
        }
    }
};

int main() {
    Chip8 myCpu;

    // --- Load a "Sprite" (A 5x5 Square) into memory at 0x050 ---
    myCpu.memory[0x50] = 0xFF; // ********
    myCpu.memory[0x51] = 0x81; // * *
    myCpu.memory[0x52] = 0x81; // * *
    myCpu.memory[0x53] = 0x81; // * *
    myCpu.memory[0x54] = 0xFF; // ********

    // --- Program: Tell the CPU to draw that sprite ---
    myCpu.memory[0x200] = 0xA0; myCpu.memory[0x201] = 0x50; // Set I to 0x050
    myCpu.memory[0x202] = 0x60; myCpu.memory[0x203] = 0x0A; // Set V0 (X pos) to 10
    myCpu.memory[0x204] = 0x61; myCpu.memory[0x205] = 0x05; // Set V1 (Y pos) to 5
    myCpu.memory[0x206] = 0xD0; myCpu.memory[0x207] = 0x15; // DRAW at (V0, V1) height 5
    myCpu.memory[0x208] = 0x12; myCpu.memory[0x209] = 0x08; // Jump to 0x208 (Halt/Loop)

    // Running the loop
    for(int i = 0; i < 5; i++) {
        myCpu.cycle();
        myCpu.draw(); // Update the graphical interface
    }

    return 0;
}