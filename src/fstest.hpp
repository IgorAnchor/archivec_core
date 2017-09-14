//
// Created by IgorTheMLGPro on 9/14/2017.
//

#ifndef ARCHIVEC_CORE_FSTEST_HPP
#define ARCHIVEC_CORE_FSTEST_HPP

#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>

namespace FSTest {

    namespace fs = boost::filesystem;

    static void test() {

        const char *path = "1.txt";

        if (fs::exists(path)) {
            if (fs::is_regular_file(path)) {
                std::cout << "regular file" << std::endl;

            } else if (fs::is_directory(path)) {
                std::cout << "dir" << std::endl;

            }
        } else {
            std::cout << "does not exist" << std::endl;

        }

    }
}

#endif //ARCHIVEC_CORE_FSTEST_HPP
