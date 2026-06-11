#include "Loader.hpp"
#include "CPU.hpp"
#include "Bus.hpp"
#include <fstream>
#include <stdexcept>

void Loader::load(CPU& cpu, const std::string& path, uint16_t origin) {
    auto& bus = cpu.getBus();
    loadToBus(bus, path, origin);

    // Legacy direct-run behavior
    cpu.state.PC = origin;
    cpu.state.SP = 0xFFFF;
    cpu.halted   = false;
}
 
void Loader::loadBytes(CPU& cpu, const std::vector<uint8_t>& bytes, uint16_t origin) {
    auto& bus = cpu.getBus();
    loadBytesToBus(bus, bytes, origin);

    cpu.state.PC = origin;
    cpu.state.SP = 0xFFFF;
    cpu.halted   = false;
}

void Loader::loadToBus(Bus& bus, const std::string& path, uint16_t origin) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("Loader: could not open file: " + path);

    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0)
        throw std::runtime_error("Loader: file is empty: " + path);

    if (static_cast<uint32_t>(origin) + static_cast<uint32_t>(fileSize) > 0x10000)
        throw std::runtime_error("Loader: file too large to fit in memory at origin 0x"
            + std::to_string(origin));

    file.seekg(0, std::ios::beg);

    uint16_t address = origin;
    char byte;
    while (file.get(byte)) {
        bus.write(address, static_cast<uint8_t>(byte));
        ++address;
    }

    if (file.bad())
        throw std::runtime_error("Loader: error while reading file: " + path);
}

void Loader::loadBytesToBus(Bus& bus, const std::vector<uint8_t>& bytes, uint16_t origin) {
    if (static_cast<uint32_t>(origin) + static_cast<uint32_t>(bytes.size()) > 0x10000)
        throw std::runtime_error("Loader: program too large to fit in memory at origin 0x"
            + std::to_string(origin));

    for (size_t i = 0; i < bytes.size(); ++i)
        bus.write(static_cast<uint16_t>(origin + i), bytes[i]);
}