#include "bitstreamwriter.hpp"
#include <cassert>
#include <cstring>

lzw::BitStreamWriter::BitStreamWriter() {
    // 8192 bits for a start (1024 bytes). It will resize if needed.
    // Default granularity is 2.
    internalInit();
    allocate(8192);
}

lzw::BitStreamWriter::BitStreamWriter(const int sizeBits, const int growthGranularity) {
    internalInit();
    setGranularity(growthGranularity);
    allocate(sizeBits);
}

lzw::BitStreamWriter::~BitStreamWriter() {
    delete[]stream;
}

void lzw::BitStreamWriter::internalInit() {
    stream = nullptr;
    bytesAllocated = 0;
    granularity = 2;
    currBytePos = 0;
    nextBitPos = 0;
    numBitsWritten = 0;
}

void lzw::BitStreamWriter::allocate(int bitsWanted) {
    // Require at least a byte.
    if (bitsWanted <= 0) {
        bitsWanted = 8;
    }

    // Round upwards if needed:
    if ((bitsWanted % 8) != 0) {
        bitsWanted = nextPowerOfTwo(bitsWanted);
    }

    // We might already have the required count.
    const int sizeInBytes = bitsWanted / 8;
    if (sizeInBytes <= bytesAllocated) {
        return;
    }

    stream = allocBytes(sizeInBytes, stream, bytesAllocated);
    bytesAllocated = sizeInBytes;
}

void lzw::BitStreamWriter::appendBit(int bit) {
    const unsigned mask = unsigned(1) << nextBitPos;
    stream[currBytePos] = static_cast<uint8_t>((stream[currBytePos] & ~mask) | (-bit & mask));
    ++numBitsWritten;

    if (++nextBitPos == 8) {
        nextBitPos = 0;
        if (++currBytePos == bytesAllocated) {
            allocate(bytesAllocated * granularity * 8);
        }
    }
}

void lzw::BitStreamWriter::appendBitU64(uint64_t num, int bitCount) {
    assert(bitCount <= 64);
    for (int b = 0; b < bitCount; ++b) {
        const uint64_t mask = uint64_t(1) << b;
        const int bit = (num & mask) != 0;
        appendBit(bit);
    }
}

uint8_t *lzw::BitStreamWriter::release() {
    uint8_t *oldPtr = stream;
    internalInit();
    return oldPtr;
}

void lzw::BitStreamWriter::setGranularity(int growthGranularity) {
    granularity = (growthGranularity >= 2) ? growthGranularity : 2;
}

unsigned lzw::BitStreamWriter::getByteCount() const {
    auto usedBytes = static_cast<unsigned int>(numBitsWritten / 8);
    int leftovers = numBitsWritten % 8;

    if (leftovers != 0) {
        ++usedBytes;
    }

    assert(usedBytes <= bytesAllocated);
    return usedBytes;
}

unsigned lzw::BitStreamWriter::getBitCount() const {
    return static_cast<unsigned int>(numBitsWritten);
}

const uint8_t *lzw::BitStreamWriter::getBitStream() const {
    return stream;
}

std::uint8_t *lzw::BitStreamWriter::allocBytes(int bytesWanted, uint8_t *oldPtr, int oldSize) {
    auto *newMemory = new uint8_t[bytesWanted];
    std::memset(newMemory, 0, static_cast<size_t>(bytesWanted));

    if (oldPtr != nullptr) {
        std::memcpy(newMemory, oldPtr, static_cast<size_t>(oldSize));
        delete[]oldPtr;
    }

    return newMemory;
}

int lzw::BitStreamWriter::nextPowerOfTwo(int num) {
    --num;
    for (size_t i = 1; i < sizeof(num) * 8; i <<= 1) {
        num = num | num >> i;
    }
    return ++num;
}
