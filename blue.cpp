#include <iostream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define RAM_LENGTH 4096
uint16_t RAM[RAM_LENGTH];

typedef uint16_t blue_register;

blue_register A;         // Accumulator
blue_register DIL;       // Data-In
blue_register DOL;       // Data-Out
blue_register DSL;       // Device Selector
blue_register IR;         // Instruction Register
blue_register MAR;       // Memory Address Register
blue_register MBR;       // Memory Buffer Register
blue_register PC = 0x00; // Program Counter
blue_register SR;         // Console Switch Register
blue_register Z;         // Z register

typedef enum {
    EXECUTE,
    FETCH,
} State;

State STATE = FETCH;

// Sample program based on your logic
// F is NOP (15), A is JMP (10)
uint16_t program0[6] = {
    0xF000, // NOP
    0xF003, // NOP
    0xA004, // JMP to 004
    0xF005, // NOP
    0xF010, // NOP (This will be skipped by the JMP)
    0x0000  // HALT (Instruction 0)
};

uint8_t get_instruction()
{
    return ((IR & 0xF000) >> 12);
}

void do_NOP(uint8_t tick)
{
    if (tick == 8)
        MAR = PC;
}

void do_JMP(uint8_t tick)
{
    if (tick == 6) {
        // In a real system, you might clear something here, 
        // but keeping your logic:
    } else if (tick == 7) {
        PC = (IR & 0x0FFF);
    } else if (tick == 8) {
        MAR = PC;
    }
}

void process_tick(uint8_t tick)
{
    switch (tick) {
    case 1:
        break;
    case 2:
        if (STATE == FETCH)
            PC += 1;
        break;
    case 3:
        if (STATE == FETCH)
            MBR = 0x00;
        break;
    case 4:
        if (STATE == FETCH) {
            IR = 0x00;
            MBR = RAM[MAR]; 
        }
        break;
    case 5:
        if (STATE == FETCH)
            IR = MBR;
        break;
    }

    uint8_t INS = get_instruction();
    if (INS == 15) { // 0xF
        do_NOP(tick);
    }
    else if (INS == 10) { // 0xA
        do_JMP(tick);
    }
}

uint8_t clock_pulse = 1;

void emulateCycle()
{
    while (clock_pulse < 9)
    {
        process_tick(clock_pulse);
        clock_pulse++;
    }
    clock_pulse = 1;
}

void dumpRegisters()
{
    printf("PC: %04x A: %04x IR: %04x MAR: %04x MBR: %04x\n", 
            PC, A, IR, MAR, MBR);
}

void runProgram(const uint16_t* program)
{
    std::cout << "Copying program to the RAM\n";
    memset(RAM, 0x00, RAM_LENGTH * sizeof(uint16_t));
    
    // Copy only the program size, not the whole RAM length from a small array!
    memcpy(RAM, program, 6 * sizeof(uint16_t)); 

    // Initialize first MAR
    MAR = PC;

    for (int i = 0; i < 20; i++) { // Run for 20 cycles so it doesn't loop forever
        emulateCycle();
        dumpRegisters();
        if (IR == 0) break; // Stop if we hit a 0000 instruction
        usleep(100000);     // Small delay to see it run
    }
}

int main(int argc, char* argv[])
{
    runProgram(program0);
    return 0;
}