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

        void    loadRom(uint16_t offset, const std::vector<uint8_t>& bytes);

        void    tick();

        RamDevice     ram;
        RomDevice     rom;
        ConsoleDevice console;
        TimerDevice   timer;

        static constexpr uint16_t IVT_BASE    = 0xFFF0;
        static constexpr uint16_t ROM_SIZE     = 0x1000;
};