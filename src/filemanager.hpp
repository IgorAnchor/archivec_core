#ifndef ARCHIVEC_CORE_FILEMANAGER_HPP
#define ARCHIVEC_CORE_FILEMANAGER_HPP

#include <fstream>
#include <vector>
#include <iterator>

#include "util/message.hpp"

namespace FileManager {

    static const long CLUSTER = 20'000'000;

    static void write_file(std::vector<char> *, const char *);

    static void open_file(const char *);


    static void open_file(const char *path) {

        std::ifstream f_in(path, std::ios::in | std::ios::binary | std::ios::ate);

        if (f_in.is_open()) {

            f_in.seekg(0, std::ios::end);
            long long size = f_in.tellg();
            f_in.seekg(0, std::ios::beg);

            const char * name = "copy.exe";

            char *buff;
            std::vector<char> *bytes_to_write;

            long long rest_size = size;

            if (size > CLUSTER) {

                long long cluster_count = size / CLUSTER;
                rest_size = size % CLUSTER;

                buff = new char[CLUSTER];
                bytes_to_write = new std::vector<char>(CLUSTER);

                for (int i = 0; i < cluster_count; ++i) {

                    f_in.read(buff, CLUSTER);
                    bytes_to_write->assign(buff, buff + CLUSTER);

                    //do compressing
                    write_file(bytes_to_write, name);

                    bytes_to_write->clear();
                }

                delete[] buff;
                delete bytes_to_write;
            }

            buff = new char[rest_size];
            bytes_to_write = new std::vector<char>(rest_size);

            bytes_to_write->resize(rest_size);

            f_in.read(buff, rest_size);
            bytes_to_write->assign(buff, buff + rest_size);

            //do compressing
            write_file(bytes_to_write, name);

            delete[] buff;
            delete bytes_to_write;

            f_in.close();
        } else {
            std::string err = "Unable to open file ";
            err.append(path);
            Message::error_message(err.c_str());
        }
    }

    static void write_file(std::vector<char> *bytes, const char *path) {

        std::ofstream f_out(path, std::ios::binary | std::ios::app);

        if (f_out.is_open()) {

            std::ostream_iterator<char> output_iterator(f_out);
            std::copy(bytes->begin(), bytes->end(), output_iterator);

            f_out.close();

        } else {
            std::string err = "Unable to write file ";
            err.append(path);
            Message::error_message(err.c_str());
        }
    }


}


#endif //ARCHIVEC_CORE_FILEMANAGER_HPP
