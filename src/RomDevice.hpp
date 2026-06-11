#ifndef ROM_DEVICE_HPP
#define ROM_DEVICE_HPP

#include <cstdint>
#include <vector>

// Simple ROM device: fixed-size read-only memory.
class RomDevice {
public:
    explicit RomDevice(std::size_t size)
        : data(size, 0xFF)
    {}

    uint8_t read(uint16_t addr) const;

    void write(uint16_t addr, uint8_t value);

    // helper for loaders: allows direct initialization
    std::vector<uint8_t>&       raw();
    const std::vector<uint8_t>& raw() const;

private:
    std::vector<uint8_t> data;
};

#endif