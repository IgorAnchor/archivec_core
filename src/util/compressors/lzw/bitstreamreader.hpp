#ifndef ARCHIVEC_CORE_BITSTREAMREADER_HPP
#define ARCHIVEC_CORE_BITSTREAMREADER_HPP

#include "bitstreamwriter.hpp"

namespace lzw {

	class BitStreamReader final {
	private:
		const uint8_t *stream;      // Pointer to the external bit stream. Not owned by the reader.
		const int sizeBytes;        // Size of the stream *in bytes*. Might include padding.
		const int sizeBits;         // Size of the stream *in bits*, padding *not* include.
		int currentBytePos;         // Current byte being read in the stream.
		int nextBitPos;             // Bit position within the current byte to access next. 0 to 7.
		int numBitsRead;

	public:
		BitStreamReader(const BitStreamReader &) = delete;
		BitStreamReader &operator=(const BitStreamReader &) = delete;
		explicit BitStreamReader(const uint8_t *bitStream, int byteCount, int bitCount);

		bool isEndOfStream() const;
		bool readNextBit(int &bitOut);
		uint64_t readBitsU64(int bitCount);
		void reset();
	};
};
#endif //ARCHIVEC_CORE_BITSTREAMREADER_HPP
