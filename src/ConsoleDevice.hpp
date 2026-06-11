#pragma once
#include <cstdint>
#include <queue>

class Bus; // if needed

class ConsoleDevice {
public:
    explicit ConsoleDevice(Bus&) {}

    // MMIO read/write entry points
    uint8_t readData();       // 0x8000
    uint8_t readStatus();     // 0x8001
    void    writeData(uint8_t v);
    void    writeControl(uint8_t v);

private:
    // For now: writes always print, reads return 0 or future keyboard data
};