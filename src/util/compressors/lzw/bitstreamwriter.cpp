#include "bitstreamwriter.hpp"
#include <cassert>
#include <cstring>

lzw::BitStreamWriter::BitStreamWriter() {
    // 8192 bits for a start (1024 bytes). It will resize if needed.
    // Default granularity is 2.
    internal_init();
    allocate(8192);
}

lzw::BitStreamWriter::BitStreamWriter(const int size_bits, const int growth_granularity) {
    internal_init();
    set_granularity(growth_granularity);
    allocate(size_bits);
}

lzw::BitStreamWriter::~BitStreamWriter() {
    delete[]stream;
}

void lzw::BitStreamWriter::internal_init() {
    stream = nullptr;
    bytes_allocated = 0;
    granularity = 2;
    curr_byte_pos = 0;
    next_bit_pos = 0;
    num_bits_written = 0;
}

void lzw::BitStreamWriter::allocate(int bits_wanted) {
    // Require at least a byte.
    if (bits_wanted <= 0) {
        bits_wanted = 8;
    }

    // Round upwards if needed:
    if ((bits_wanted % 8) != 0) {
        bits_wanted = next_power_of_two(bits_wanted);
    }

    // We might already have the required count.
    const int sizeInBytes = bits_wanted / 8;
    if (sizeInBytes <= bytes_allocated) {
        return;
    }

    stream = alloc_bytes(sizeInBytes, stream, bytes_allocated);
    bytes_allocated = sizeInBytes;
}

void lzw::BitStreamWriter::append_bit(int bit) {
    const uint32_t mask = uint32_t (1) << next_bit_pos;
    stream[curr_byte_pos] = static_cast<uint8_t>((stream[curr_byte_pos] & ~mask) | (-bit & mask));
    ++num_bits_written;

    if (++next_bit_pos == 8) {
        next_bit_pos = 0;
        if (++curr_byte_pos == bytes_allocated) {
            allocate(bytes_allocated * granularity * 8);
        }
    }
}

void lzw::BitStreamWriter::append_bit_u64(uint64_t num, int bit_count) {
    assert(bit_count <= 64);
    for (int b = 0; b < bit_count; ++b) {
        const uint64_t mask = uint64_t(1) << b;
        const int bit = (num & mask) != 0;
        append_bit(bit);
    }
}

uint8_t *lzw::BitStreamWriter::release() {
    uint8_t *old_ptr = stream;
    internal_init();
    return old_ptr;
}

void lzw::BitStreamWriter::set_granularity(int growth_granularity) {
    granularity = (growth_granularity >= 2) ? growth_granularity : 2;
}

int lzw::BitStreamWriter::get_byte_count() const {
    int usedBytes = num_bits_written / 8;
    int leftovers = num_bits_written % 8;

    if (leftovers != 0) {
        ++usedBytes;
    }

    assert(usedBytes <= bytes_allocated);
    return usedBytes;
}

int lzw::BitStreamWriter::get_bit_count() const {
    return num_bits_written;
}

const uint8_t *lzw::BitStreamWriter::get_bit_stream() const {
    return stream;
}

std::uint8_t *lzw::BitStreamWriter::alloc_bytes(int bytes_wanted, uint8_t *old_ptr, int old_size) {
    uint8_t *newMemory = new uint8_t[bytes_wanted];
    std::memset(newMemory, 0, bytes_wanted);

    if (old_ptr != nullptr) {
        std::memcpy(newMemory, old_ptr, old_size);
        delete[]old_ptr;
    }

    return newMemory;
}

int lzw::BitStreamWriter::next_power_of_two(int num) {
    --num;
    for (size_t i = 1; i < sizeof(num) * 8; i <<= 1) {
        num = num | num >> i;
    }
    return ++num;
}
