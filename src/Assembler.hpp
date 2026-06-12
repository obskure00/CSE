#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace Assembler {

    struct Result {
        std::vector<uint8_t> bytes;
        uint16_t             origin;
    };

    Result   assembleFile(const std::string& path);

    // Legacy wrapper: returns only the byte vector (origin is in the bytes themselves if .ORG is used)
    std::vector<uint8_t> assemble(const std::string& path);
}

#endif