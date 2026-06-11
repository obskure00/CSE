#include "CPU.hpp"
#include "Instructions.hpp"
#include "Bus.hpp"
#include <iostream>

#include <iostream>
#include <iomanip>

void CPU::checkInterrupts() {
    if (interruptsEnabled && bus.timer.irqPending()) {
        bus.timer.clearIrq();

        uint16_t handler = 0xF100;

        stackPush16(state.PC);
        interruptsEnabled = false;
        state.PC = handler;
    }
}

void CPU::reset() {
    state.PC = 0xF000;
    state.SP = 0xFFFF;
    halted   = false;
    interruptsEnabled = true;
    pendingIRQs = 0;
}

Bus& CPU::getBus() {
    return bus;
}

uint8_t CPU::fetchByte() {
    return bus.read(state.PC++);
}

void CPU::fetchRegPair(uint8_t& rd, uint8_t& rs) {
    uint8_t byte = fetchByte();
    rd = (byte >> 4) & 0x07;
    rs =  byte       & 0x07;
}

void CPU::stackPush(uint8_t value) {
    bus.write(state.SP, value);
    state.SP--;
}

uint8_t CPU::stackPop() {
    state.SP++;
    return bus.read(state.SP);
}

void CPU::stackPush16(uint16_t value) {
    stackPush(static_cast<uint8_t>(value >> 8));
    stackPush(static_cast<uint8_t>(value & 0xFF));
}

uint16_t CPU::stackPop16() {
    uint8_t lo = stackPop();
    uint8_t hi = stackPop();
    return (static_cast<uint16_t>(hi) << 8) | lo;
}

void CPU::setArithmeticFlags(uint16_t result, uint8_t a, uint8_t b, bool subtract) {
    uint8_t r8   = static_cast<uint8_t>(result);
    state.SR.Z   = (r8 == 0);
    state.SR.S   = (r8 & 0x80) != 0;
    state.SR.C   = (result > 0xFF);
    if (subtract)
        state.SR.V = ((a ^ b) & 0x80) != 0 && ((a ^ r8) & 0x80) != 0;
    else
        state.SR.V = ((~(a ^ b)) & 0x80) != 0 && ((a ^ r8) & 0x80) != 0;
}

void CPU::setLogicFlags(uint8_t result) {
    state.SR.Z = (result == 0);
    state.SR.S = (result & 0x80) != 0;
    state.SR.C = false;
    state.SR.V = false;
}

bool CPU::step() {
    if (halted) return false;

    // poll interrupts
    checkInterrupts();

    uint8_t opByte = fetchByte();
    return execute(*this, static_cast<Opcode>(opByte));
}

void CPU::run(uint64_t maxCycles) {
    uint64_t cycles = 0;
    while (!halted) {
        if (!step()) break;
        bus.tick();
        if (maxCycles > 0 && ++cycles >= maxCycles) break;
    }
}