#include "Loader.hpp"
#include "CPU.hpp"
#include "Bus.hpp"
#include <fstream>
#include <stdexcept>

void Loader::load(CPU& cpu, const std::string& path, uint16_t origin) {
    loadToBus(cpu.getBus(), path, origin);
    cpu.state.PC = origin;
    cpu.state.SP = 0xEFFF;
    cpu.halted   = false;
}

void Loader::loadBytes(CPU& cpu, const std::vector<uint8_t>& bytes, uint16_t origin) {
    loadBytesToBus(cpu.getBus(), bytes, origin);
    cpu.state.PC = origin;
    cpu.state.SP = 0xEFFF;
    cpu.halted   = false;
}

void Loader::loadToBus(Bus& bus, const std::string& path, uint16_t origin) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("Loader: could not open file: " + path);

    auto fileSize = file.tellg();
    if (fileSize <= 0)
        throw std::runtime_error("Loader: file is empty: " + path);

    if (static_cast<uint32_t>(origin) + static_cast<uint32_t>(fileSize) > 0x10000)
        throw std::runtime_error("Loader: file too large to fit in memory at origin 0x"
            + std::to_string(origin));

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(bytes.data()), fileSize);
    if (file.bad())
        throw std::runtime_error("Loader: error while reading file: " + path);

    loadBytesToBus(bus, bytes, origin);
}

void Loader::loadBytesToBus(Bus& bus, const std::vector<uint8_t>& bytes, uint16_t origin) {
    if (static_cast<uint32_t>(origin) + static_cast<uint32_t>(bytes.size()) > 0x10000)
        throw std::runtime_error("Loader: program too large to fit in memory at origin 0x"
            + std::to_string(origin));

    if (origin >= 0xF000) {
        bus.loadRom(static_cast<uint16_t>(origin - 0xF000), bytes);
        return;
    }

    for (size_t i = 0; i < bytes.size(); ++i) {
        uint16_t addr = static_cast<uint16_t>(origin + i);
        if (addr >= 0xF000) {
            std::vector<uint8_t> romPart(bytes.begin() + static_cast<ptrdiff_t>(i), bytes.end());
            bus.loadRom(static_cast<uint16_t>(addr - 0xF000), romPart);
            return;
        }
        bus.write(addr, bytes[i]);
    }
}