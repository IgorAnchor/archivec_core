#include <cstdio>
#include "dictionary.hpp"

lzw::Dictionary::Dictionary() {
	// First 256 dictionary entries are reserved to the byte/ASCII
	// range. Additional entries follow for the character sequences
	// found in the input. Up to 4096 - 256 (MaxDictEntries - FIRST_CODE).
	size = static_cast<unsigned >(FIRST_CODE);
	for (int32_t i = 0; i < size; ++i) {
		entries[i].code = NIL;
		entries[i].value = i;
	}
}

int lzw::Dictionary::findIndex(int code, int value) const {
	if (code == NIL) {
		return value;
	}
	for (int32_t i = 0; i < size; ++i) {
		if (entries[i].code == code && entries[i].value == value) {
			return i;
		}
	}
	return NIL;
}

bool lzw::Dictionary::add(const int code, const int value) {
	if (size == MAX_DICT_ENTRIES) {
		fprintf(stderr, "Dictionary overflowed!\n");
		return false;
	}

	entries[size].code = code;
	entries[size].value = value;
	++size;
	return true;
}

bool lzw::Dictionary::flush(int &codeWidthBits) {
	if (size == (1 << codeWidthBits)) {
		++codeWidthBits;
		if (codeWidthBits > MAX_DICT_BITS) {
			// Clear the dictionary (except the first 256 byte entries).
			codeWidthBits = START_BITS;
			size = static_cast<unsigned >(FIRST_CODE);
			return true;
		}
	}
	return false;
}