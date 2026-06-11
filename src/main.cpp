#include "Loader.hpp"
#include "CPU.hpp"
#include "Bus.hpp"
#include <Assembler.hpp>
#include <fstream>
#include <stdexcept>
#include <iostream>

static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: emulator.exe <program.bin>\n";
        return 1;
    }

    Bus bus;
    CPU cpu(bus);
    std::string path = argv[1];

    try {
        if (endsWith(path, ".txt") || endsWith(path, ".asm")) {
            auto bytes = Assembler::assemble(path);
            Loader::loadBytes(cpu, bytes);
        } else {
            Loader::load(cpu, path);
        }
        cpu.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }



    // Loader::load(cpu, argv[1]);
    // cpu.run();
    return 0;
}