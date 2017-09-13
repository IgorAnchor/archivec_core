#ifndef ARCHIVEC_CORE_FILEMANAGER_HPP
#define ARCHIVEC_CORE_FILEMANAGER_HPP

#include <fstream>
#include <vector>
#include <iterator>

#include "util/message.hpp"

namespace FileManager {

    static void write_file(std::vector<char> *, const char *);

    static std::vector<char> *open_file(const char *);


    static std::vector<char> *open_file(const char *path) {

        std::vector<char> *buffer;

        std::ifstream f_in(path, std::ios::in | std::ios::binary | std::ios::ate);

        if (f_in.is_open()) {

            f_in.seekg(0, std::ios::end);
            long long size = f_in.tellg();
            f_in.seekg(0, std::ios::beg);

            buffer = new std::vector<char>(size);
            buffer->assign(std::istreambuf_iterator<char>(f_in), std::istreambuf_iterator<char>());

            f_in.close();

//            for (auto &&byte :*buffer) {
//                if (byte == '\n')
//                    std::cout << std::endl;
//                else
//                    std::cout << byte;
//            }

            return buffer;

        } else {
            std::string err = "Unable to open file ";
            err.append(path);
            Message::error_message(err.c_str());
        }
    }

    static void write_file(std::vector<char> *bytes, const char *path) {

        std::ofstream f_out(path, std::ios::binary);

        if (f_out.is_open()) {

            std::ostream_iterator<char> output_iterator(f_out);
            std::copy(bytes->begin(), bytes->end(), output_iterator);

            f_out.close();

        } else {
            std::string err = "Unable to open file ";
            err.append(path);
            Message::error_message(err.c_str());
        }
    }


}


#endif //ARCHIVEC_CORE_FILEMANAGER_HPP
