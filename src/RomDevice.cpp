#include "RomDevice.hpp"
#include <cstdint>

uint8_t RomDevice::read(uint16_t addr) const {
    if (addr < data.size()) {
        return data[addr];
    } else {
        // Out of bounds reads return 0xFF (open bus behavior).
        return 0xFF;
    }
}

void RomDevice::write(uint16_t addr, uint8_t value) {
    // Ignore writes to ROM. You could assert/log here in debug builds.
    (void)addr;
    (void)value;
}

// helper for loaders: allows direct initialization
std::vector<uint8_t>& RomDevice::raw() { return data; }
const std::vector<uint8_t>& RomDevice::raw() const { return data; }