#include <iostream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <unistd.h> 
#include <stdlib.h> 
#include <ctime>    

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
    uint8_t keypad[16]; 
    uint8_t delay_timer;
    bool drawFlag; // NEW: Tells main loop when to refresh the screen

    Chip8() {
        pc = 0x200;
        I = 0;
        sp = 0;
        delay_timer = 0;
        drawFlag = true;
        memset(memory, 0, MEMORY_SIZE);
        memset(V, 0, 16);
        memset(display, false, sizeof(display));
        memset(keypad, 0, 16);
        srand(time(NULL)); 
    }

    bool loadROM(const char* filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streampos size = file.tellg();
            char* buffer = new char[size];
            file.seekg(0, std::ios::beg);
            file.read(buffer, size);
            file.close();
            for (long i = 0; i < size; ++i) {
                memory[0x200 + i] = (uint8_t)buffer[i];
            }
            delete[] buffer;
            return true;
        }
        return false;
    }

    void draw() {
        printf("\033[H"); // Cursor to home
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x++) {
                printf(display[y * DISPLAY_WIDTH + x] ? "##" : "  ");
            }
            printf("\n");
        }
    }

    void cycle() {
        uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
        decode(opcode);
    }

    void updateTimers() {
        if (delay_timer > 0) delay_timer--;
    }

    void decode(uint16_t opcode) {
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;
        uint8_t nn = (opcode & 0x00FF);
        uint16_t nnn = (opcode & 0x0FFF);

        switch(opcode & 0xF000) {
            case 0x0000:
                if (opcode == 0x00E0) { // Clear Screen
                    memset(display, false, sizeof(display));
                    drawFlag = true;
                    pc += 2;
                } else if (opcode == 0x00EE) { // Return
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                }
                break;
            case 0x1000: pc = nnn; break; // Jump
            case 0x2000: // Call
                stack[sp] = pc;
                sp++;
                pc = nnn;
                break;
            case 0x3000: // Skip if VX == NN
                if (V[x] == nn) pc += 4;
                else pc += 2;
                break;
            case 0x4000: // Skip if VX != NN
                if (V[x] != nn) pc += 4;
                else pc += 2;
                break;
            case 0x6000: V[x] = nn; pc += 2; break; // Set VX
            case 0x7000: V[x] += nn; pc += 2; break; // Add VX
            
            case 0x8000: // Math instructions (Pong needs these!)
                switch(opcode & 0x000F) {
                    case 0x0: V[x] = V[y]; break;
                    case 0x2: V[x] &= V[y]; break;
                    case 0x4: // Add with carry
                        V[0xF] = (V[y] > (0xFF - V[x])) ? 1 : 0;
                        V[x] += V[y];
                        break;
                    case 0x5: // Sub
                        V[0xF] = (V[x] > V[y]) ? 1 : 0;
                        V[x] -= V[y];
                        break;
                }
                pc += 2;
                break;

            case 0xA000: I = nnn; pc += 2; break;
            case 0xC000: V[x] = (rand() % 256) & nn; pc += 2; break;
            case 0xD000: { // Draw
                uint8_t vx = V[x] % DISPLAY_WIDTH;
                uint8_t vy = V[y] % DISPLAY_HEIGHT;
                uint8_t height = opcode & 0x000F;
                V[0xF] = 0;
                for (int row = 0; row < height; row++) {
                    uint8_t sprite_byte = memory[I + row];
                    for (int col = 0; col < 8; col++) {
                        if ((sprite_byte & (0x80 >> col)) != 0) {
                            int idx = (vx + col) + ((vy + row) * DISPLAY_WIDTH);
                            if (idx < DISPLAY_WIDTH * DISPLAY_HEIGHT) {
                                if (display[idx]) V[0xF] = 1; 
                                display[idx] ^= true;
                            }
                        }
                    }
                }
                drawFlag = true; // Signal a redraw is needed
                pc += 2;
                break;
            }
            case 0xE000: pc += 2; break; // Skip keys for now
            case 0xF000:
                if (nn == 0x07) { V[x] = delay_timer; pc += 2; }
                else if (nn == 0x15) { delay_timer = V[x]; pc += 2; }
                else if (nn == 0x1E) { I += V[x]; pc += 2; }
                else { pc += 2; }
                break;
            default: pc += 2; break;
        }
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ./chip8 \"ROM_NAME.ch8\"\n");
        return 1;
    }

    Chip8 myCpu;
    if (!myCpu.loadROM(argv[1])) {
        printf("Could not find ROM file!\n");
        return 1;
    }

    printf("\033[2J");   // Clear screen
    printf("\033[?25l"); // Hide cursor (stops a lot of flashing!)

    while (true) {
        // Run several CPU cycles per "frame"
        for(int i = 0; i < 10; i++) {
            myCpu.cycle();
        }

        myCpu.updateTimers();

        // ONLY DRAW IF THE CPU CHANGED THE SCREEN
        if (myCpu.drawFlag) {
            myCpu.draw();
            myCpu.drawFlag = false;
        }

        usleep(16000); // 60 FPS
    }

    return 0;
}