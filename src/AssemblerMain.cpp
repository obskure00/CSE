#include "Assembler.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: assembler <input.txt> <output.bin>\n";
        return 1;
    }

    try {
        auto result = Assembler::assembleFile(argv[1]);
        std::cout << "origin=0x" << std::hex << result.origin
                  << " size=" << std::dec << result.bytes.size() << " bytes\n";

        std::ofstream out(argv[2], std::ios::binary);
        if (!out) {
            std::cerr << "Error: could not open output file: " << argv[2] << "\n";
            return 1;
        }
        out.write(reinterpret_cast<const char*>(result.bytes.data()),
                  static_cast<std::streamsize>(result.bytes.size()));

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}