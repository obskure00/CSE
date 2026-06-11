#include "Memory.hpp"

uint8_t Memory::read(uint16_t address) {
    return RAM[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    RAM[address] = value;
}