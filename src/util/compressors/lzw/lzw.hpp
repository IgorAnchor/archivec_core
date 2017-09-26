#ifndef ARCHIVEC_CORE_LZW_HPP
#define ARCHIVEC_CORE_LZW_HPP

#include "dictionary.hpp"
#include "bitstreamwriter.hpp"
#include "bitstreamreader.hpp"

namespace lzw {

    static bool output_sequence(const Dictionary &, int, uint8_t *&, uint32_t &, int &);

    static bool output_byte(int, uint8_t *&, uint32_t &);

    static void compress(const uint8_t *uncompressed, uint32_t uncompressed_size_bytes,
                  uint8_t **compressed, uint32_t *compressed_size_bytes, uint32_t *compressed_size_bits) {
        // LZW encoding context:
        int code = NIL;
        int code_bits_width = START_BITS;
        Dictionary dictionary;

        // Output bit stream we write to. This will allocate
        // memory as needed to accommodate the encoded data.
        BitStreamWriter bit_stream;

        for (; uncompressed_size_bytes > 0; --uncompressed_size_bytes, ++uncompressed) {
            const int value = *uncompressed;
            const int index = dictionary.find_index(code, value);

            if (index != NIL) {
                code = index;
                continue;
            }

            // Write the dictionary code using the minimum bit-with:
            bit_stream.append_bit_u64(static_cast<uint64_t>(code), code_bits_width);

            // Flush it when full so we can restart the sequences.
            if (!dictionary.flush(code_bits_width)) {
                // There's still space for this sequence.
                dictionary.add(code, value);
            }
            code = value;
        }

        // Residual code at the end:
        if (code != NIL) {
            bit_stream.append_bit_u64(static_cast<uint64_t>(code), code_bits_width);
        }

        // Pass ownership of the compressed data buffer to the user pointer:
        *compressed_size_bytes = bit_stream.get_byte_count();
        *compressed_size_bits = bit_stream.get_bit_count();
        *compressed = bit_stream.release();
    }

    static uint32_t expand(const uint8_t *compressed, const uint32_t compressed_size_bytes, const uint32_t compressed_size_bits,
               uint8_t *uncompressed) {
        int code;
        int prev_code = NIL;
        int first_byte = 0;
        uint32_t bytes_decoded = 0;
        int code_bits_width = START_BITS;


        // We'll reconstruct the dictionary based on the
        // bit stream codes. Unlike Huffman encoding, we
        // don't store the dictionary as a prefix to the data.
        Dictionary dictionary;
        BitStreamReader bit_stream(compressed, compressed_size_bytes, compressed_size_bits);

        // We check to avoid an overflow of the user buffer.
        // If the buffer is smaller than the decompressed size,
        // LZW_ERROR() is called. If that doesn't throw or
        // terminate we break the loop and return the current
        // decompression count.
        while (!bit_stream.is_end_of_stream()) {
            assert(code_bits_width <= MAX_DICT_BITS);
            code = static_cast<int>(bit_stream.read_bits_u64(code_bits_width));

            if (prev_code == NIL) {
                if (!output_byte(code, uncompressed, bytes_decoded)) {
                    break;
                }
                first_byte = code;
                prev_code = code;
                continue;
            }

            if (code >= dictionary.size) {
                if (!output_sequence(dictionary, prev_code, uncompressed, bytes_decoded, first_byte)) {
                    break;
                }
                if (!output_byte(first_byte, uncompressed, bytes_decoded)) {
                    break;
                }
            } else {
                if (!output_sequence(dictionary, code, uncompressed, bytes_decoded, first_byte)) {
                    break;
                }
            }

            dictionary.add(prev_code, first_byte);
            if (dictionary.flush(code_bits_width)) {
                prev_code = NIL;
            } else {
                prev_code = code;
            }
        }

        return bytes_decoded;
    }


    static bool output_byte(int code, uint8_t *&output, uint32_t &bytes_decoded_so_far) {
        assert(code >= 0 && code < 256);
        *output++ = static_cast<std::uint8_t>(code);
        ++bytes_decoded_so_far;
        return true;
    }

    static bool output_sequence(const Dictionary &dict, int code, uint8_t *&output, uint32_t &bytes_decoded_so_far,
                                int &first_byte) {
        // A sequence is stored backwards, so we have to write
        // it to a temp then output the buffer in reverse.
        int i = 0;
        std::uint8_t sequence[MAX_DICT_ENTRIES];
        do {
            assert(i < MAX_DICT_ENTRIES - 1 && code >= 0);
            sequence[i++] = static_cast<uint8_t>(dict.entries[code].value);
            code = dict.entries[code].code;
        } while (code >= 0);

        first_byte = sequence[--i];
        for (; i >= 0; --i) {
            if (!output_byte(sequence[i], output, bytes_decoded_so_far)) {
                return false;
            }
        }
        return true;
    }

};

#endif //ARCHIVEC_CORE_LZW_HPP
