#ifndef TIMER_DEVICE_HPP
#define TIMER_DEVICE_HPP

#include <cstdint>

// Register map (base 0x8010):
//   +0  counter lo  (R/W)
//   +1  counter hi  (R/W)
//   +2  compare lo  (R/W, writing enables timer)
//   +3  compare hi  (R/W, writing enables timer)
//   +4  control     (R/W)
//         bit 0 = enable
//         bit 1 = auto-reload (periodic); 0 = one-shot
//         bit 7 = IRQ pending (read-only); write 1 to clear
class TimerDevice {
public:
    TimerDevice() = default;

    std::uint8_t readReg(std::uint8_t index);
    void         writeReg(std::uint8_t index, std::uint8_t value);
    void         tick();

    bool irqPending() const { return irq; }
    void clearIrq()         { irq = false; }

private:
    std::uint16_t counter  = 0;
    std::uint16_t compare  = 0;
    bool          enabled  = false;
    bool          autoReload = false;
    bool          irq      = false;
};

#endif