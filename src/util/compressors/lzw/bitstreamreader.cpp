#include "bitstreamreader.hpp"
#include <cassert>

lzw::BitStreamReader::BitStreamReader(const uint8_t *bit_stream, const int32_t byte_count, const int32_t bit_count)
        : stream(bit_stream),
          size_bytes(byte_count),
          size_bits(bit_count) {
    reset();
}

bool lzw::BitStreamReader::read_next_bot(int32_t &bit_out) {
    if (num_bits_read >= size_bits) {
        return false; // We are done.
    }

    const uint32_t mask = uint32_t(1) << next_bit_pos;
    bit_out = (stream[current_byte_pos] & mask) != 0;
    ++num_bits_read;

    if (++next_bit_pos == 8) {
        next_bit_pos = 0;
        ++current_byte_pos;
    }
    return true;
}

uint64_t lzw::BitStreamReader::read_bits_u64(int32_t bitCount) {
    assert(bitCount <= 64);

    uint64_t num = 0;
    for (int32_t b = 0; b < bitCount; ++b) {
        int32_t bit;
        if (!read_next_bot(bit)) {
//            LZW_ERROR("Failed to read bits from stream! Unexpected end.");
            break;
        }

        // Based on a "Stanford bit-hack":
        // http://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
        const uint64_t mask = uint64_t(1) << b;
        num = (num & ~mask) | (-bit & mask);
    }

    return num;
}

void lzw::BitStreamReader::reset() {
    current_byte_pos = 0;
    next_bit_pos = 0;
    num_bits_read = 0;
}

bool lzw::BitStreamReader::is_end_of_stream() const {
    return num_bits_read >= size_bits;
}