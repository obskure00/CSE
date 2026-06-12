#ifndef CPU_HPP
#define CPU_HPP

#include "Registers.hpp"
#include <cstdint>

class Bus;

class CPU {
    private:
        Bus& bus;

    public:
        explicit CPU(Bus& b) : bus(b) {}

        CPUState state;
        bool halted = false;

        bool    interruptsEnabled = true;
        uint8_t pendingIRQs       = 0;

        void requestInterrupt(uint8_t line) { pendingIRQs |= static_cast<uint8_t>(1u << line); }
        void checkInterrupts();

        void reset();

        Bus& getBus();

        bool step();
        void run(uint64_t maxCycles = 0);

        uint8_t  fetchByte();
        void     fetchRegPair(uint8_t& rd, uint8_t& rs);

        void     stackPush(uint8_t value);
        uint8_t  stackPop();
        void     stackPush16(uint16_t value);
        uint16_t stackPop16();

        void setArithmeticFlags(uint16_t result, uint8_t a, uint8_t b, bool subtract);
        void setLogicFlags(uint8_t result);
};

#endif