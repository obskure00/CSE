#pragma once
#include <cstdint>
#include <array>
#include <string>

// Simple block-storage device, backed by a host file ("disk image").
//
// Register map (MMIO base 0x8020):
//   +0  BLOCK_HI   (R/W) high byte of target block number
//   +1  BLOCK_LO   (R/W) low byte of target block number
//   +2  OFFSET     (R/W) position (0-255) within the 256-byte sector buffer
//   +3  DATA       (R/W) read/write a byte at OFFSET in the buffer;
//                   OFFSET auto-increments (wraps 255 -> 0) on each access
//   +4  CMD/STATUS (W: 1=load block into buffer, 2=store buffer to block,
//                       3=clear buffer to zero)
//                  (R: 0 = ok, 1 = error / block out of range)
//   +5  DISKSZ_HI  (R) total number of blocks on the disk, high byte
//   +6  DISKSZ_LO  (R) total number of blocks on the disk, low byte
//
// The host file is created (zero-filled) if it does not already exist.
class DiskDevice {
public:
    static constexpr size_t BLOCK_SIZE = 256;
    static constexpr size_t NUM_BLOCKS = 128;

    explicit DiskDevice(const std::string& path);

    uint8_t readReg(uint8_t index);
    void    writeReg(uint8_t index, uint8_t value);

private:
    std::string path_;
    std::array<uint8_t, BLOCK_SIZE> buffer_{};
    uint16_t blockNum_ = 0;
    uint8_t  offset_   = 0;
    uint8_t  status_   = 0;

    void ensureFile();
    void loadBlock();
    void storeBlock();
};