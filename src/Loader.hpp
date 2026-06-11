#ifndef LOADER_HPP
#define LOADER_HPP
 
#include "CPU.hpp"
#include <cstdint>
#include <string>
#include <vector>
 
class Loader {
public:
    static void load(CPU& cpu, const std::string& path, uint16_t origin = 0x0000);
    static void loadBytes(CPU& cpu, const std::vector<uint8_t>& bytes, uint16_t origin = 0x0000);

    static void loadToBus(Bus& bus, const std::string& path, uint16_t origin);
    static void loadBytesToBus(Bus& bus, const std::vector<uint8_t>& bytes, uint16_t origin);
};
 
#endif