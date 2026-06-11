#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP
 
#include <string>
#include <vector>
#include <cstdint>
 
namespace Assembler {
    std::vector<uint8_t> assemble(const std::string& path);
}
 
#endif