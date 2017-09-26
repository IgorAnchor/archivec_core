#include "dictionary.hpp"

lzw::Dictionary::Dictionary() {
    // First 256 dictionary entries are reserved to the byte/ASCII
    // range. Additional entries follow for the character sequences
    // found in the input. Up to 4096 - 256 (MaxDictEntries - FIRST_CODE).
    size = FIRST_CODE;
    for (int32_t i = 0; i < size; ++i) {
        entries[i].code = NIL;
        entries[i].value = i;
    }
}

int lzw::Dictionary::find_index(int32_t code, int32_t value) const {
    if (code == NIL) {
        return value;
    }

    // Linear search for now.
    // TODO: Worth optimizing with a proper hash-table?
    for (int32_t i = 0; i < size; ++i) {
        if (entries[i].code == code && entries[i].value == value) {
            return i;
        }
    }

    return NIL;
}

bool lzw::Dictionary::add(const int32_t code, const int32_t value) {
    if (size == MAX_DICT_ENTRIES) {
//        LZW_ERROR("Dictionary overflowed!");
        return false;
    }

    entries[size].code = code;
    entries[size].value = value;
    ++size;
    return true;
}

bool lzw::Dictionary::flush(int32_t &code_width_bits) {
    if (size == (1 << code_width_bits)) {
        ++code_width_bits;
        if (code_width_bits > MAX_DICT_BITS) {
            // Clear the dictionary (except the first 256 byte entries).
            code_width_bits = START_BITS;
            size = FIRST_CODE;
            return true;
        }
    }
    return false;
}