#ifndef ARCHIVEC_CORE_LZW_HPP
#define ARCHIVEC_CORE_LZW_HPP

#include "dictionary.hpp"
#include "bitstreamwriter.hpp"
#include "bitstreamreader.hpp"

namespace lzw {

	static bool outputSequence(const Dictionary &, int, uint8_t *&, unsigned &, int &);
	static bool outputByte(int, uint8_t *&, unsigned &);

	static void compress(const uint8_t *uncompressed, unsigned uncompressedSizeBytes,
	                     uint8_t **compressed, unsigned *compressedSizeBytes, unsigned *compressedSizeBits) {
		// LZW encoding context:
		int code = NIL;
		int codeWidthBits = START_BITS;
		Dictionary dictionary;

		// Output bit stream we write to. This will allocate
		// memory as needed to accommodate the encoded data.
		BitStreamWriter bitStream;

		for (; uncompressedSizeBytes > 0; --uncompressedSizeBytes, ++uncompressed) {
			const int value = *uncompressed;
			const int index = dictionary.findIndex(code, value);

			if (index != NIL) {
				code = index;
				continue;
			}

			// Write the dictionary code using the minimum bit-with:
			bitStream.appendBitU64(static_cast<uint64_t>(code), codeWidthBits);

			// Flush it when full so we can restart the sequences.
			if (!dictionary.flush(codeWidthBits)) {
				// There's still space for this sequence.
				dictionary.add(code, value);
			}
			code = value;
		}

		// Residual code at the end:
		if (code != NIL) {
			bitStream.appendBitU64(static_cast<uint64_t>(code), codeWidthBits);
		}

		// Pass ownership of the compressed data buffer to the user pointer:
		*compressedSizeBytes = bitStream.getByteCount();
		*compressedSizeBits = bitStream.getBitCount();
		*compressed = bitStream.release();
	}

	static unsigned expand(const uint8_t *compressed, const unsigned compressedSizeBytes, const unsigned compressedSizeBits,
	                       uint8_t *uncompressed) {
		int code;
		int prevCode = NIL;
		int firstByte = 0;
		unsigned bytesDecoded = 0;
		int codeWidthBits = START_BITS;


		// We'll reconstruct the dictionary based on the
		// bit stream codes. Unlike Huffman encoding, we
		// don't store the dictionary as a prefix to the data.
		Dictionary dictionary;
		BitStreamReader bit_stream(compressed, compressedSizeBytes, compressedSizeBits);

		// We check to avoid an overflow of the user buffer.
		// If the buffer is smaller than the decompressed size,
		// LZW_ERROR() is called. If that doesn't throw or
		// terminate we break the loop and return the current
		// decompression count.
		while (!bit_stream.isEndOfStream()) {
			assert(codeWidthBits <= MAX_DICT_BITS);
			code = static_cast<int>(bit_stream.readBitsU64(codeWidthBits));

			if (prevCode == NIL) {
				if (!outputByte(code, uncompressed, bytesDecoded)) {
					break;
				}
				firstByte = code;
				prevCode = code;
				continue;
			}

			if (code >= dictionary.size) {
				if (!outputSequence(dictionary, prevCode, uncompressed, bytesDecoded, firstByte)) {
					break;
				}
				if (!outputByte(firstByte, uncompressed, bytesDecoded)) {
					break;
				}
			} else {
				if (!outputSequence(dictionary, code, uncompressed, bytesDecoded, firstByte)) {
					break;
				}
			}

			dictionary.add(prevCode, firstByte);
			if (dictionary.flush(codeWidthBits)) {
				prevCode = NIL;
			} else {
				prevCode = code;
			}
		}

		return bytesDecoded;
	}


	static bool outputByte(int code, uint8_t *&output, unsigned &bytesDecodedSoFar) {
		assert(code >= 0 && code < 256);
		*output++ = static_cast<std::uint8_t>(code);
		++bytesDecodedSoFar;
		return true;
	}

	static bool outputSequence(const Dictionary &dict, int code, uint8_t *&output, unsigned &bytesDecodedSoFar, int &firstByte) {
		// A sequence is stored backwards, so we have to write
		// it to a temp then output the buffer in reverse.
		int i = 0;
		uint8_t sequence[MAX_DICT_ENTRIES];
		do {
			assert(i < MAX_DICT_ENTRIES - 1 && code >= 0);
			sequence[i++] = static_cast<uint8_t>(dict.entries[code].value);
			code = dict.entries[code].code;
		} while (code >= 0);

		firstByte = sequence[--i];
		for (; i >= 0; --i) {
			if (!outputByte(sequence[i], output, bytesDecodedSoFar)) {
				return false;
			}
		}
		return true;
	}

};

#endif //ARCHIVEC_CORE_LZW_HPP
