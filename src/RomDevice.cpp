#include "RomDevice.hpp"
#include <cstdint>

uint8_t RomDevice::read(uint16_t addr) const {
    if (addr < data.size()) {
        return data[addr];
    } else {
        return 0xFF;
    }
}

void RomDevice::write(uint16_t addr, uint8_t value) {
    (void)addr;
    (void)value;
}

std::vector<uint8_t>& RomDevice::raw() { return data; }
const std::vector<uint8_t>& RomDevice::raw() const { return data; }