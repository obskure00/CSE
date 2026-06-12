#include "Loader.hpp"
#include "CPU.hpp"
#include "Bus.hpp"
#include "Assembler.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static bool isTextAsm(const std::string& path) {
    return endsWith(path, ".txt") || endsWith(path, ".asm");
}

static std::string findFile(const std::string& name, const fs::path& exeDir) {
    if (fs::exists(name)) return name;
    fs::path candidate = exeDir / name;
    if (fs::exists(candidate)) return candidate.string();
    throw std::runtime_error("could not find file: " + name +
        "\n  searched: " + name +
        "\n  searched: " + candidate.string());
}

int main(int argc, char* argv[]) {
    fs::path exeDir = fs::path(argv[0]).parent_path();

    std::string kernelPath  = "kernel.txt";
    std::string bootromPath = "bootrom.txt";

    if (argc >= 2) kernelPath  = argv[1];
    if (argc >= 3) bootromPath = argv[2];

    try {
        kernelPath  = findFile(kernelPath,  exeDir);
        bootromPath = findFile(bootromPath, exeDir);

        Bus bus;
        CPU cpu(bus);

        // Load boot ROM into 0xF000-0xFFFF
        if (isTextAsm(bootromPath)) {
            auto result = Assembler::assembleFile(bootromPath);
            Loader::loadBytesToBus(bus, result.bytes, result.origin);
        } else {
            Loader::loadToBus(bus, bootromPath, 0xF000);
        }

        if (isTextAsm(kernelPath)) {
            auto result = Assembler::assembleFile(kernelPath);
            Loader::loadBytesToBus(bus, result.bytes, result.origin);
        } else {
            Loader::loadToBus(bus, kernelPath, 0x0200);
        }

        cpu.reset();
        cpu.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}