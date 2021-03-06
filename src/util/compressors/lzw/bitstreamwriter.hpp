#ifndef ARCHIVEC_CORE_BITSTREAMWRITER_HPP
#define ARCHIVEC_CORE_BITSTREAMWRITER_HPP

#include <cstdint>
#include <string>

namespace lzw {

    class BitStreamWriter final {
    private:
        uint8_t *stream;       // buffer to store our bits. Heap allocated & owned by the class instance.
        int bytesAllocated;    // Current size of heap-allocated stream buffer *in bytes*.
        int currBytePos;       // Current byte being written to, from 0 to bytesAllocated-1.
        int nextBitPos;        // Bit position within the current byte to access next. 0 to 7.
        int numBitsWritten;    // Number of bits in use from the stream buffer, not including byte-rounding padding.

    public:
        BitStreamWriter(const BitStreamWriter &) = delete;
        BitStreamWriter &operator=(const BitStreamWriter &) = delete;
        explicit BitStreamWriter();
        ~BitStreamWriter();

        void allocate(int bitsWanted);
        uint8_t *release();
        void appendBit(int bit);
        void appendBitU64(uint64_t num, int bitCount);
	    unsigned getByteCount() const;
	    unsigned getBitCount() const;

    private:
        void internalInit();
        static uint8_t *allocBytes(int bytesWanted, uint8_t *oldPtr, int oldSize);
        static inline int nextPowerOfTwo(int num);
    };
};

#endif //ARCHIVEC_CORE_BITSTREAMWRITER_HPP
