#include <iostream>

#include "src/archiver.hpp"


int main(int argc, char *argv[]) {

    Archiver *archiver = new Archiver("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/test/");

    std::vector<std::string_view >s;
    s.emplace_back("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/test/a.txt");
    s.emplace_back("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/test/y.txt");

    archiver->add_to_archive(s);
    archiver->crush("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");


    std::vector<ArchivedFile> entries = archiver->extract_files_info(
            "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");

    for (auto &&entry : entries) {
        std::cout << entry.size << "  " << entry.name << "   " << entry.id << std::endl;

    }

    archiver->extract("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar",
                      "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/out/");

    return 0;
}


