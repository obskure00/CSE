#ifndef CPU_HPP
#define CPU_HPP

#include "Registers.hpp"
#include <cstdint>

class Bus;

class CPU {
    private:
        Bus& bus;

    public:
        CPU(Bus& bus) : bus(bus) {}

        CPUState state;
        bool halted = false;

        Bus& getBus();

        bool step();
        void run(uint64_t MaxCycles = 0);

        uint8_t fetchByte();
        void    fetchRegPair(uint8_t& rd, uint8_t& rs);

        void     stackPush(uint8_t value);
        uint8_t  stackPop();
        void     stackPush16(uint16_t value);
        uint16_t stackPop16();

        void setArithmeticFlags(uint16_t result, uint8_t a, uint8_t b, bool subtract);
        void setLogicFlags(uint8_t result);
};

#endif