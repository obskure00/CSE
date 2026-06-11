#include <cstdint>
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

        void    tick();    // call in CPU::run

        RamDevice     ram;
        RomDevice     rom;
        ConsoleDevice console;
        TimerDevice   timer;

    private:
        static constexpr uint16_t ROM_SIZE  = 0x1000; // 4 KiB
};