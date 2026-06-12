#include <cstdint>
#include "Memory.hpp"
#include "RamDevice.hpp"



uint8_t RamDevice::read(uint16_t addr) {
    return mem.read(addr);
}

void RamDevice::write(uint16_t addr, uint8_t value) {
    mem.write(addr, value);
}