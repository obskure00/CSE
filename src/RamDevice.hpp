#ifndef RAMDEVICE_HPP
#define RAMDEVICE_HPP

#include <cstdint>
#include "Memory.hpp"

// Simple RAM device: wraps existing Memory storage.
// For now, assumes full 64 KiB; you can later parameterize size.
class RamDevice {
public:
    RamDevice() = default;

    // If you want smaller RAM later, you can add a size and range checks.
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

private:
    Memory mem;
};

#endif