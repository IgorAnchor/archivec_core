#include <iostream>

#include "src/archiver.hpp"

int main(int argc, char *argv[]) {
    Archiver *archiver = new Archiver("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/test/");
    archiver->crush("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");


    uint32_t f_count = archiver->extract_files_count(
            "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");
    std::cout << "Count: " << f_count << std::endl;


    std::vector<std::string> names = archiver->extract_files_names(
            "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");

    std::cout << "Contains:" << std::endl;
    for (auto &&item : names) {
        std::cout << "\t" << item << std::endl;
    }


    archiver->extract_file(
            "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar",
            "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/2/123.exe", 1);

    archiver->extract("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar",
                      "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/3/");
    return 0;
}
