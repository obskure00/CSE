#pragma once
#include <cstdint>
#include <queue>

class Bus;

// MMIO console device.
//   0x8000  data  (R: pop next input byte, 0 if none / W: print char)
//   0x8001  status (R: bit0 = output ready (always 1)
//                       bit1 = input available)
class ConsoleDevice {
public:
    explicit ConsoleDevice(Bus&);
    ~ConsoleDevice();

    uint8_t readData();
    uint8_t readStatus();
    void    writeData(uint8_t v);
    void    writeControl(uint8_t v);

private:
    void poll();

    std::queue<uint8_t> inputQueue;

#ifndef _WIN32
    bool rawModeEnabled = false;
    void enableRawMode();
    void disableRawMode();
#endif
};