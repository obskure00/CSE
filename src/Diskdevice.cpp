#include "DiskDevice.hpp"
#include <fstream>
#include <vector>

DiskDevice::DiskDevice(const std::string& path) : path_(path) {
    ensureFile();
}

void DiskDevice::ensureFile() {
    std::ifstream check(path_, std::ios::binary);
    if (check.good()) return;

    std::ofstream create(path_, std::ios::binary);
    std::vector<char> zeros(BLOCK_SIZE * NUM_BLOCKS, 0);
    create.write(zeros.data(), static_cast<std::streamsize>(zeros.size()));
}

void DiskDevice::loadBlock() {
    if (blockNum_ >= NUM_BLOCKS) { status_ = 1; return; }

    std::ifstream in(path_, std::ios::binary);
    in.seekg(static_cast<std::streamoff>(blockNum_) * static_cast<std::streamoff>(BLOCK_SIZE));
    in.read(reinterpret_cast<char*>(buffer_.data()), static_cast<std::streamsize>(BLOCK_SIZE));

    auto got = static_cast<size_t>(in.gcount());
    for (size_t i = got; i < BLOCK_SIZE; ++i) buffer_[i] = 0;

    status_ = 0;
}

void DiskDevice::storeBlock() {
    if (blockNum_ >= NUM_BLOCKS) { status_ = 1; return; }

    std::fstream out(path_, std::ios::binary | std::ios::in | std::ios::out);
    out.seekp(static_cast<std::streamoff>(blockNum_) * static_cast<std::streamoff>(BLOCK_SIZE));
    out.write(reinterpret_cast<const char*>(buffer_.data()), static_cast<std::streamsize>(BLOCK_SIZE));
    out.flush();

    status_ = 0;
}

uint8_t DiskDevice::readReg(uint8_t index) {
    switch (index) {
    case 0: return static_cast<uint8_t>((blockNum_ >> 8) & 0xFF);
    case 1: return static_cast<uint8_t>(blockNum_ & 0xFF);
    case 2: return offset_;
    case 3: {
        uint8_t v = buffer_[offset_];
        offset_ = static_cast<uint8_t>(offset_ + 1);
        return v;
    }
    case 4: return status_;
    case 5: return static_cast<uint8_t>((NUM_BLOCKS >> 8) & 0xFF);
    case 6: return static_cast<uint8_t>(NUM_BLOCKS & 0xFF);
    default: return 0xFF;
    }
}

void DiskDevice::writeReg(uint8_t index, uint8_t value) {
    switch (index) {
    case 0:
        blockNum_ = static_cast<uint16_t>((blockNum_ & 0x00FF) | (static_cast<uint16_t>(value) << 8));
        break;
    case 1:
        blockNum_ = static_cast<uint16_t>((blockNum_ & 0xFF00) | value);
        break;
    case 2:
        offset_ = value;
        break;
    case 3:
        buffer_[offset_] = value;
        offset_ = static_cast<uint8_t>(offset_ + 1);
        break;
    case 4:
        if (value == 1) loadBlock();
        else if (value == 2) storeBlock();
        else if (value == 3) { buffer_.fill(0); status_ = 0; }
        break;
    default:
        break;
    }
}