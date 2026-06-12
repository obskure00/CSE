#include "ConsoleDevice.hpp"
#include <iostream>

uint8_t ConsoleDevice::readData() {
    return 0;
}

uint8_t ConsoleDevice::readStatus() {
    return 0x01;
}

void ConsoleDevice::writeData(uint8_t v) {
    std::cout << static_cast<char>(v);
    std::cout.flush();
}

void ConsoleDevice::writeControl(uint8_t) {
    // maybe clear screen, etc. later
}