#include "bitstreamreader.hpp"
#include <cassert>

lzw::BitStreamReader::BitStreamReader(const uint8_t *bitStream, const int byteCount, const int bitCount)
		: stream(bitStream),
		  sizeBytes(byteCount),
		  sizeBits(bitCount) {
	reset();
}

bool lzw::BitStreamReader::readNextBit(int &bitOut) {
	if (numBitsRead >= sizeBits) {
		return false;
	}

	const int mask = int(1) << nextBitPos;
	bitOut = (stream[currentBytePos] & mask) != 0;
	++numBitsRead;

	if (++nextBitPos == 8) {
		nextBitPos = 0;
		++currentBytePos;
	}
	return true;
}

uint64_t lzw::BitStreamReader::readBitsU64(int bitCount) {
	assert(bitCount <= 64);

	uint64_t num = 0;
	for (int b = 0; b < bitCount; ++b) {
		int bit;
		if (!readNextBit(bit)) {
			fprintf(stderr, "Failed to read bits from stream! Unexpected end.\n");
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
	currentBytePos = 0;
	nextBitPos = 0;
	numBitsRead = 0;
}

bool lzw::BitStreamReader::isEndOfStream() const {
	return numBitsRead >= sizeBits;
}