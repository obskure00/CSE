#include "Loader.hpp"
#include "CPU.hpp"
#include "Bus.hpp"
#include <Assembler.hpp>
#include <fstream>
#include <stdexcept>
#include <iostream>

static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main(int argc, char* argv[]) {
    // Optional argument: kernel source/binary path
    // If none given, default to "kernel.txt"
    std::string kernelPath = "kernel.txt";
    if (argc >= 2) {
        kernelPath = argv[1];
    }

    try {
        Bus bus;
        CPU cpu(bus);

        // -------------------------------------------------
        // 1. Load kernel into RAM at 0x0200
        //    Kernel must start with 'B','K' magic at 0x0200
        // -------------------------------------------------
        if (endsWith(kernelPath, ".txt") || endsWith(kernelPath, ".asm")) {
            auto bytes = Assembler::assemble(kernelPath);
            Loader::loadBytesToBus(bus, bytes, 0x0200);
        } else {
            Loader::loadToBus(bus, kernelPath, 0x0200);
        }

        // -------------------------------------------------
        // 2. Load boot ROM into ROM at 0xF000
        //    bootrom.bin should match your bootrom.txt
        // -------------------------------------------------
        Loader::loadToBus(bus, "bootrom.bin", 0xF000);

        // -------------------------------------------------
        // 3. Reset CPU: PC -> 0xF000, SP init, etc.
        //    Then run: BootROM -> Kernel -> Shell
        // -------------------------------------------------
        cpu.reset();
        cpu.run();   // you can pass a maxCycles if you want

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}