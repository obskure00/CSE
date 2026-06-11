#ifndef TIMER_DEVICE_HPP
#define TIMER_DEVICE_HPP

#include <cstdint>

class TimerDevice {
public:
    TimerDevice() = default;

    std::uint8_t readReg(std::uint8_t index);
    void         writeReg(std::uint8_t index, std::uint8_t value);
    void         tick();

    bool irqPending() const { return irq; }
    void clearIrq()         { irq = false; }

private:
    std::uint16_t counter = 0;
    std::uint16_t compare = 0;
    bool          enabled = false;
    bool          irq     = false;
};

#endif