#ifndef ARCHIVEC_CORE_DICTIONARY_HPP
#define ARCHIVEC_CORE_DICTIONARY_HPP

#include <cstdint>

namespace lzw {

    constexpr int NIL = -1;
    constexpr int MAX_DICT_BITS = 10;
    constexpr int START_BITS = 9;
    constexpr int FIRST_CODE = (1 << (START_BITS - 1)); // 256
    constexpr int MAX_DICT_ENTRIES = (1 << MAX_DICT_BITS);     // 4096

    class Dictionary final {
    public:

        struct Entry {
            int code;
            int value;
        };

        int32_t size;
        Entry entries[MAX_DICT_ENTRIES];

    public:
        Dictionary();

        int32_t find_index(int32_t code, int32_t value) const;

        bool add(int32_t code, int32_t value);

        bool flush(int32_t &code_width_bits);
    };

};
#endif //ARCHIVEC_CORE_DICTIONARY_HPP
