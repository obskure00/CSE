#include "Bus.hpp"
#include <cstring>

uint8_t Bus::read(uint16_t addr) {
    if (addr >= 0xF000) {
        return rom.read(static_cast<uint16_t>(addr - 0xF000));
    } else if (addr >= 0x8000 && addr <= 0x80FF) {
        // MMIO window
        if (addr == 0x8000) return console.readData();
        if (addr == 0x8001) return console.readStatus();
        if (addr >= 0x8010 && addr <= 0x8014) {
            return timer.readReg(static_cast<uint8_t>(addr - 0x8010));
        }
        return 0xFF;
    } else {
        return ram.read(addr);
    }
}

void Bus::write(uint16_t addr, uint8_t value) {
    if (addr >= 0xF000) {
        // ROM writes are silently ignored (use loadRom() to initialise)
        return;
    } else if (addr >= 0x8000 && addr <= 0x80FF) {
        if (addr == 0x8000) { console.writeData(value);   return; }
        if (addr == 0x8001) { console.writeControl(value); return; }
        if (addr >= 0x8010 && addr <= 0x8014) {
            timer.writeReg(static_cast<uint8_t>(addr - 0x8010), value);
        }
        // other MMIO: ignore
    } else {
        ram.write(addr, value);
    }
}

void Bus::loadRom(uint16_t offset, const std::vector<uint8_t>& bytes) {
    auto& raw = rom.raw();
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (offset + i < raw.size())
            raw[offset + i] = bytes[i];
    }
}

void Bus::tick() {
    timer.tick();
}