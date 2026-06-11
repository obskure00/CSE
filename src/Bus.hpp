#ifndef BUS_HPP
#define BUS_HPP

#include <cstdint>
#include "RamDevice.hpp"
#include "RomDevice.hpp"

class Bus {
    public:
        Bus()
            : ram(), rom(ROM_SIZE)
        {}

        uint8_t read(uint16_t addr);
        void    write(uint16_t addr, uint8_t value);

        RamDevice ram;
        RomDevice rom;

    private:
        // static constexpr uint16_t RAM_SIZE = 0xF000; // 60 KiB
        static constexpr uint16_t ROM_SIZE = 0x1000; // 4 KiB
};

#endif