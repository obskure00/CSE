#ifndef RAMDEVICE_HPP
#define RAMDEVICE_HPP

#include <cstdint>
#include "Memory.hpp"

class RamDevice {
public:
    RamDevice() = default;

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

private:
    Memory mem;
};

#endif