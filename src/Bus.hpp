#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "RamDevice.hpp"
#include "RomDevice.hpp"
#include "ConsoleDevice.hpp"
#include "TimerDevice.hpp"
#include "DiskDevice.hpp"

class Bus {
    public:
        explicit Bus(const std::string& diskPath = "disk.img")
            : ram(), rom(ROM_SIZE), console(*this), timer(), disk(diskPath) {}

        uint8_t read(uint16_t addr);
        void    write(uint16_t addr, uint8_t value);

        void    loadRom(uint16_t offset, const std::vector<uint8_t>& bytes);

        void    tick();

        RamDevice     ram;
        RomDevice     rom;
        ConsoleDevice console;
        TimerDevice   timer;
        DiskDevice    disk;

        static constexpr uint16_t IVT_BASE    = 0x0000;
        static constexpr uint16_t ROM_SIZE     = 0x1000;
};