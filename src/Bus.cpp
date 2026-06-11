#include "Bus.hpp"

uint8_t Bus::read(uint16_t addr) {
    if (addr < 0x8000) {
        return ram.read(addr);
    } else if (addr >= 0x8000 && addr <= 0x80FF) {
        // MMIO window
        if (addr == 0x8000) return console.readData();
        if (addr == 0x8001) return console.readStatus();

        if (addr >= 0x8010 && addr <= 0x8013) {
            uint8_t index = static_cast<uint8_t>(addr - 0x8010);
            return timer.readReg(index);
        }
        return 0xFF;
    } else if (addr >= 0xF000) {
        uint16_t romAddr = addr - 0xF000;
        return rom.read(romAddr);
    } else {
        // rest of RAM
        return ram.read(addr);
    }
}

void Bus::write(uint16_t addr, uint8_t value) {
    if (addr < 0x8000) {
        ram.write(addr, value);
    } else if (addr >= 0x8000 && addr <= 0x80FF) {
        if (addr == 0x8000) { console.writeData(value); return; }
        if (addr == 0x8001) { console.writeControl(value); return; }

        if (addr >= 0x8010 && addr <= 0x8013) {
            uint8_t index = static_cast<uint8_t>(addr - 0x8010);
            timer.writeReg(index, value);
            return;
        }
        // ignore other MMIO writes for now
    } else if (addr >= 0xF000) {
        // writes to ROM ignored
    } else {
        ram.write(addr, value);
    }
}

void Bus::tick() {
    timer.tick();
}