#include <iostream>
#include <stdint.h>
#include <vector>

// CHIP-8 standard sizes
#define MEMORY_SIZE 4096
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

class Chip8 {
public:
    uint8_t  memory[MEMORY_SIZE];
    uint8_t  V[16];          // Registers V0-VF
    uint16_t I;              // Index register
    uint16_t pc;             // Program Counter
    
    uint16_t stack[16];
    uint8_t  sp;             // Stack Pointer

    bool display[DISPLAY_WIDTH * DISPLAY_HEIGHT];

    Chip8() {
        pc = 0x200;          // CHIP-8 programs start at 0x200
        I = 0;
        sp = 0;
        // Clear memory and display here...
    }

    void cycle() {
        // 1. Fetch Opcode (2 bytes)
        uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

        // 2. Decode & Execute
        // This is where we replace your Blue logic with CHIP-8 instructions
        decode(opcode);
    }

    void decode(uint16_t opcode) {
        uint8_t first_nibble = (opcode & 0xF000) >> 12;

        switch(first_nibble) {
            case 0x1: // 1NNN: Jump to NNN
                pc = (opcode & 0x0FFF);
                break;
            
            case 0x6: // 6XNN: Set VX to NN
                V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
                pc += 2;
                break;

            // More instructions go here...
            default:
                pc += 2; 
                break;
        }
    }
};

int main() {
    Chip8 myCpu;

    // A tiny CHIP-8 "Program" manually loaded into memory
    // 6005 -> Set Register V0 to 5
    // 610A -> Set Register V1 to 10
    // 1200 -> Jump back to 0x200 (Infinite Loop)
    myCpu.memory[0x200] = 0x60; myCpu.memory[0x201] = 0x05;
    myCpu.memory[0x202] = 0x61; myCpu.memory[0x203] = 0x0A;
    myCpu.memory[0x204] = 0x12; myCpu.memory[0x205] = 0x00;

    std::cout << "Starting CHIP-8..." << std::endl;

    for(int i = 0; i < 10; i++) { // Run for 10 cycles
        // Print status before the cycle
        printf("PC: 0x%03X | Opcode: %02X%02X | V0: %d | V1: %d\n", 
                myCpu.pc, myCpu.memory[myCpu.pc], myCpu.memory[myCpu.pc+1], myCpu.V[0], myCpu.V[1]);
        
        myCpu.cycle();
    }

    return 0;
}