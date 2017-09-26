#ifndef ARCHIVEC_CORE_BITSTREAMREADER_HPP
#define ARCHIVEC_CORE_BITSTREAMREADER_HPP

#include "bitstreamwriter.hpp"

namespace lzw {

    class BitStreamReader final {
    private:
        const uint8_t *stream;          // Pointer to the external bit stream. Not owned by the reader.
        const int32_t size_bytes;       // Size of the stream *in bytes*. Might include padding.
        const int32_t size_bits;        // Size of the stream *in bits*, padding *not* include.
        int32_t current_byte_pos;       // Current byte being read in the stream.
        int32_t next_bit_pos;           // Bit position within the current byte to access next. 0 to 7.
        int32_t num_bits_read;

    public:
        // No copy/assignment.
        BitStreamReader(const BitStreamReader &) = delete;

        BitStreamReader &operator=(const BitStreamReader &) = delete;

        explicit BitStreamReader(const uint8_t *bit_stream, int32_t byte_count, int32_t bit_count);

        bool is_end_of_stream() const;

        bool read_next_bot(int32_t &bit_out);

        uint64_t read_bits_u64(int32_t bitCount);

        void reset();
    };
};
#endif //ARCHIVEC_CORE_BITSTREAMREADER_HPP
