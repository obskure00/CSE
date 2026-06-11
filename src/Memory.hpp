#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstddef>
#include <cstdint>
#include <array>

class Memory {
    private:
        static constexpr size_t SIZE = 65536;
        std::array<uint8_t, SIZE> RAM;
    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
};

#endif