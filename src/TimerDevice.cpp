#include "TimerDevice.hpp"

std::uint8_t TimerDevice::readReg(std::uint8_t index) {
    switch (index) {
    case 0: return static_cast<std::uint8_t>(counter & 0x00FF);
    case 1: return static_cast<std::uint8_t>((counter >> 8) & 0xFF);
    case 2: return static_cast<std::uint8_t>(compare & 0x00FF);
    case 3: return static_cast<std::uint8_t>((compare >> 8) & 0xFF);
    case 4: {
        uint8_t ctrl = 0;
        if (enabled)    ctrl |= 0x01;
        if (autoReload) ctrl |= 0x02;
        if (irq)        ctrl |= 0x80;
        return ctrl;
    }
    default: return 0;
    }
}

void TimerDevice::writeReg(std::uint8_t index, std::uint8_t value) {
    switch (index) {
    case 0: counter = static_cast<std::uint16_t>((counter & 0xFF00) | value); break;
    case 1: counter = static_cast<std::uint16_t>((counter & 0x00FF) | (std::uint16_t(value) << 8)); break;
    case 2: compare = static_cast<std::uint16_t>((compare & 0xFF00) | value); break;
    case 3: compare = static_cast<std::uint16_t>((compare & 0x00FF) | (std::uint16_t(value) << 8)); break;
    case 4:
        enabled    = (value & 0x01) != 0;
        autoReload = (value & 0x02) != 0;
        if (value & 0x80) irq = false;
        break;
    default: break;
    }
}

void TimerDevice::tick() {
    if (!enabled) return;
    ++counter;
    if (counter == compare) {
        irq = true;
        if (autoReload) {
            counter = 0;
        } else {
            enabled = false;
        }
    }
}