#include "Assembler.hpp"
#include <fstream>
#include <iostream>
 
int main(int argc, char** argv) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: assembler <input.asm> <output.bin>\n";
            return 1;
        }
 
        auto bytes = Assembler::assemble(argv[1]);
 
        std::ofstream outFile(argv[2], std::ios::binary);
        if (!outFile) throw std::runtime_error("could not open output file");
        outFile.write(reinterpret_cast<const char*>(bytes.data()),
                      static_cast<std::streamsize>(bytes.size()));
 
        std::cout << "Wrote " << bytes.size() << " bytes to " << argv[2] << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Assembler error: " << e.what() << "\n";
        return 1;
    }
}