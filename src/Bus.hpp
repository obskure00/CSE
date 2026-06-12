#pragma once
#include <cstdint>
#include <vector>
#include "RamDevice.hpp"
#include "RomDevice.hpp"
#include "ConsoleDevice.hpp"
#include "TimerDevice.hpp"

class Bus {
    public:
        Bus()
            : ram(), rom(ROM_SIZE), console(*this), timer() {}

        uint8_t read(uint16_t addr);
        void    write(uint16_t addr, uint8_t value);

        // Direct ROM initialisation – bypasses the bus write (ROM is read-only)
        void    loadRom(uint16_t offset, const std::vector<uint8_t>& bytes);

        void    tick();    // call each CPU cycle

        RamDevice     ram;
        RomDevice     rom;
        ConsoleDevice console;
        TimerDevice   timer;

        // Interrupt vector table base (default 0x0000, in RAM)
        // Layout: [0x0000..0x0001] = timer IRQ vector (hi, lo)
        // Placed in RAM (not ROM) so the kernel can install its own
        // handler address at startup via STR.
        static constexpr uint16_t IVT_BASE    = 0x0000;
        static constexpr uint16_t ROM_SIZE     = 0x1000; // 4 KiB (0xF000-0xFFFF)
};