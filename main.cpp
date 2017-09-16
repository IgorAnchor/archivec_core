#include <iostream>

#include "src/archiver.hpp"


int main(int argc, char *argv[]) {
    Archiver *archiver = new Archiver();

    std::vector<std::string_view> s;
    s.push_back("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/test/x.txt");
    s.push_back("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/test/y.txt");
    s.push_back("aye.txt");
    s.push_back("test");
    s.push_back("test1");

    //archiver->add_to_archive(s);
    archiver->crush("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");


    std::vector<std::string_view> s1;
    s1.emplace_back("test2/1.txt");
    s1.emplace_back("test2/2.txt");

    archiver->add_to_existing_archive(s1, "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");


    /*std::vector<ArchivedFile> entries = archiver->extract_files_info(
            "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar");

    for (auto &&entry : entries) {
        std::cout << entry.size << "  " << entry.name << "   " << entry.id << std::endl;

    }*/

    archiver->extract("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/test.ar",
                      "C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/out/");



//    fs::remove_all("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/1/");
    fs::remove_all("C:/Users/IgorTheMLGPro/CLionProjects/3-1/archivec-core/cmake-build-debug/out/");

    return 0;
}


