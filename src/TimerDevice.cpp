#include "TimerDevice.hpp"

std::uint8_t TimerDevice::readReg(std::uint8_t index) {
    switch (index) {
    case 0: return static_cast<std::uint8_t>(counter & 0x00FF);
    case 1: return static_cast<std::uint8_t>((counter >> 8) & 0xFF);
    case 2: return static_cast<std::uint8_t>(compare & 0x00FF);
    case 3: return static_cast<std::uint8_t>((compare >> 8) & 0xFF);
    default: return 0;
    }
}

void TimerDevice::writeReg(std::uint8_t index, std::uint8_t value) {
    switch (index) {
    case 0: counter = (counter & 0xFF00) | value; break;
    case 1: counter = (counter & 0x00FF) | (std::uint16_t(value) << 8); break;
    case 2: compare = (compare & 0xFF00) | value; enabled = true; break;
    case 3: compare = (compare & 0x00FF) | (std::uint16_t(value) << 8); enabled = true; break;
    default: break;
    }
}

void TimerDevice::tick() {
    if (!enabled) return;
    ++counter;
    if (counter == compare) {
        irq = true;
    }
}