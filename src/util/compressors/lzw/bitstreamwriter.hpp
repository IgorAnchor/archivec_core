#ifndef ARCHIVEC_CORE_BITSTREAMWRITER_HPP
#define ARCHIVEC_CORE_BITSTREAMWRITER_HPP

#include <cstdint>
#include <string>

namespace lzw {

    class BitStreamWriter final {
    private:
        uint8_t *stream;           // Growable buffer to store our bits. Heap allocated & owned by the class instance.
        int bytes_allocated;    // Current size of heap-allocated stream buffer *in bytes*.
        int granularity;       // Amount bytesAllocated multiplies by when auto-resizing in appendBit().
        int curr_byte_pos;       // Current byte being written to, from 0 to bytesAllocated-1.
        int next_bit_pos;        // Bit position within the current byte to access next. 0 to 7.
        int num_bits_written;    // Number of bits in use from the stream buffer, not including byte-rounding padding.

    public:

        // No copy/assignment.
        BitStreamWriter(const BitStreamWriter &) = delete;

        BitStreamWriter &operator=(const BitStreamWriter &) = delete;

        BitStreamWriter();

        ~BitStreamWriter();

        explicit BitStreamWriter(int size_bits, int growth_granularity = 2);

        void allocate(int bits_wanted);

        void set_granularity(int growth_granularity);

        std::uint8_t *release();

        void append_bit(int bit);

        void append_bit_u64(uint64_t num, int bit_count);

        int get_byte_count() const;

        int get_bit_count() const;

        const uint8_t *get_bit_stream() const;

    private:

        void internal_init();

        static uint8_t *alloc_bytes(int bytes_wanted, uint8_t *old_ptr, int old_size);

        static inline int next_power_of_two(int num);

    };
};

#endif //ARCHIVEC_CORE_BITSTREAMWRITER_HPP
