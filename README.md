# mini-emulator

A CHIP-8 emulator written in C++.

Parses and executes CHIP-8 opcodes — memory management, the call stack, timers, and display rendering — and outputs the display directly to the terminal using ASCII characters. Includes several classic CHIP-8 ROMs (Pong variants) for testing.

---

## Build

```bash
g++ -o chip8 chip8.cpp
```

## Run

```bash
./chip8 "Pong (1 player).ch8"
```

The display renders directly in the terminal and refreshes continuously.

Press `Ctrl+C` to exit.

---

## Status

| Component | Status |
|---|---|
| Opcode decoding | Working |
| Memory management | Working |
| Call stack | Working |
| Display / render loop | Working |
| Timers | Working |
| Keyboard input | Not yet implemented |

ROMs that depend on player input (e.g. Pong's paddle controls) will run and render, but cannot currently be controlled.

---

## Notes

`main.cpp` is a separate file used for local environment testing and is not part of the emulator itself.
