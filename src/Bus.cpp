#include "Bus.hpp"

uint8_t Bus::read(uint16_t addr) {
    if (addr < 0xF000) {
        return ram.read(addr);
    } else {
        uint16_t romAddr = addr - 0xF000;
        return rom.read(romAddr);
    }
}

void Bus::write(uint16_t addr, uint8_t value) {
    if (addr < 0xF000) {
        ram.write(addr, value);
    } else {
        uint16_t romAddr = addr - 0xF000;
        rom.write(romAddr, value);
    }
}