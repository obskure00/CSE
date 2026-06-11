#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <cstdint>

class CPU;

enum class Opcode : uint8_t {
    NOP = 0x00,

    // Data transfer
    MOV  = 0x10,
    LDI  = 0x11,
    LDR  = 0x12,
    STR  = 0x13,

    // ALU
    ADD  = 0x20,
    SUB  = 0x21,
    AND  = 0x22,
    OR   = 0x23,
    XOR  = 0x24,
    NOT  = 0x25,
    SHL  = 0x26,
    SHR  = 0x27,
    CMP  = 0x28,

    // Control flow
    JMP  = 0x30,
    JZ   = 0x31,
    JNZ  = 0x32,
    JC   = 0x33,
    JN   = 0x34,
    JV   = 0x35,

    // Subroutines
    CALL = 0x40,
    RET  = 0x41,

    // Stack
    PUSH = 0x50,
    POP  = 0x51,

    // Output
    OUT  = 0x60,
    OUTN = 0x61,

    HLT  = 0xFF,
};

bool execute(CPU& cpu, Opcode op);

#endif