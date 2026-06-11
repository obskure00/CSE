#include <cstdint>

struct Flags {
    bool Z = false;     // Zero flag
    bool C = false;     // Carry flag
    bool V = false;     // Signed arithmetic overflow
    bool S = false;     // Result is negative
};

struct CPUState {
    uint8_t R[8]{};

    uint16_t PC = 0;
    uint16_t SP = 0;

    Flags SR{};
};